#include "cx_socket_tcp.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include <dirent.h>
#include <netdb.h>
#include "../loger/cx_loger.h"

namespace cx
{
	cxSocketTcp::cxSocketTcp()
	{
		
	}
	cxSocketTcp::~cxSocketTcp()
	{
		Close();
	}
	std::shared_ptr<cxSocketTcp> cxSocketTcp::CreateSocket(SocketFD fd)
	{
		if (!GInit())
		{
			cxLogerError("创建socket失败，初始化底层socket服务失败");
			return NULL;
		}

		Ptr ret = std::make_shared<cxSocketTcp>();
		ret->m_fd = fd;
		if (ret->m_fd == ERROR_SOCKET)
		{
			ret->m_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ret->m_fd == ERROR_SOCKET)
			{
				cxLogerError("创建socket失败，errno: " << errno);
				return NULL;
			}
		}

		g_sockets.set(ret->m_fd, ret);

		//ret->m_binit = true;
		return ret;
	}
	SocketFD cxSocketTcp::GetFD()
	{
		return m_fd;
	}
	std::string cxSocketTcp::GetIp()
	{
		return m_ip;
	}
	int cxSocketTcp::GetPort()
	{
		return m_port;
	}	
	bool cxSocketTcp::IsConnected()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (/*!m_binit || */!m_bconnected)
		{
			return false;
		}
		return true;
	}
	void cxSocketTcp::Close()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		return _Close();
	}
	void cxSocketTcp::SetOnClose(CBF_OnClose cb)
	{
		m_cb_onclose = cb;
	}
	void cxSocketTcp::SetTimeout(unsigned int time_out)
	{
		m_timeout = time_out;
	}
	bool cxSocketTcp::Send(void* data, unsigned int data_size)
	{
		std::unique_lock<std::mutex> Lock(m_mutex_send);

		if (/*!m_binit || */!m_bconnected)
		{
			return false;
		}

		unsigned int len = 0;
		while (len < data_size)
		{
			int send_len = send(m_fd, data + len, data_size - len, MSG_NOSIGNAL | MSG_DONTWAIT);
			m_tick = cxCommonFun::GetTickTime();
			if (send_len < 0)
			{
				if (errno == EINTR) // 遇到了中断（可能是debug导致）
				{
					continue;
				}
				else if (errno == EAGAIN) // 写入缓冲区满了
				{
					// 睡眠100微秒，此数值是根据默认linux缓冲区大小，并且结合千兆网络最大速度得出
					std::this_thread::sleep_for(std::chrono::microseconds(100));
					continue;
				}
				else // socket发生了错误
				{
					//cxLogerError("socket发送数据时遇到了错误，errno：" << errno);
					Close();
					return false;
				}
			}
			else if (send_len == 0)
			{
				Close();
				return false;
			}
			else
			{
				len += send_len;
			}
		}

		return true;
	}
	bool cxSocketTcp::Recv(void* buf, unsigned int buf_size, int& recv_size)
	{
		std::unique_lock<std::mutex> Lock(m_mutex_recv);

		if (!IsConnected())
		{
			return false;
		}

		while (IsConnected())
		{
			recv_size = recv(m_fd, buf, buf_size, MSG_NOSIGNAL | MSG_DONTWAIT);
			m_tick = cxCommonFun::GetTickTime();
			if (recv_size < 0)
			{
				if (errno == EINTR) // 遇到了中断（可能是debug导致）
				{
					continue;
				}
				else if (errno == EAGAIN) // 写入缓冲区满了
				{
					// 睡眠100微秒，此数值是根据默认linux缓冲区大小，并且结合千兆网络最大速度得出
					std::this_thread::sleep_for(std::chrono::microseconds(100));
					continue;
				}
				else
				{
					Close();
					return false;
				}
			}
			else if (recv_size == 0)
			{
				Close();
				return false;
			}
			return true;
		}

		return false;
	}
	bool cxSocketTcp::BeginRecv(CBF_OnRecv cb)
	{
		if (!IsConnected())
		{
			return false;
		}

		m_cb_onrecv = cb;

		m_tick = cxCommonFun::GetTickTime();

		if (!_BindEpoll(EPOLLIN | EPOLLET | EPOLLONESHOT))
		{
			Close();
			return false;
		}

		return true;
	}
	bool cxSocketTcp::Connect(const std::string& ip, int port, unsigned int time_out)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (time_out == 0)
		{
			return _Connect(ip, port);
		}
		else
		{
			cxThreadPool::RunThread(_Thread_Connect, shared_from_this(), ip, port);
			if (!m_sem_connect.WaitOne(time_out))
			{
				_Close();
				return false;
			}
			if (!m_bconnected)
			{
				_Close();
				return false;
			}
			return true;
		}
	}
	bool cxSocketTcp::BeginListen(const std::string& ip, int port, CBF_OnAccept cb)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);

		if (m_bconnected)
		{
			cxLogerError("socket异步启动服务端失败，socket已连接");
			return false;
		}

		if (m_blisten)
		{
			cxLogerError("socket异步启动服务端失败，socket已经开启监听");
			return false;
		}

		// 创建ip端口参数
		struct sockaddr_in ServerAddress;
		memset((char*)&ServerAddress, 0, sizeof(ServerAddress));
		ServerAddress.sin_family = AF_INET;
		if (ip == "0.0.0.0" || ip == "")
		{
			ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		else
		{
			ServerAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		}
		ServerAddress.sin_port = htons(port);

		// 防止前一个程序占用端口
		int bReuseaddr = 1;
		setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(int));

		// 将客户端设置为非阻塞模式
		int flags = fcntl(m_fd, F_GETFL, 0);
		if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			_Close();
			cxLogerError("socket异步启动服务端失败，绑定时发生了错误，可能是ip地址错误或此端口已被占用，errno: " << errno);
			return false;
		}

		// 绑定ip端口
		if (::bind(m_fd, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
		{
			_Close();
			cxLogerError("socket异步启动服务端失败，绑定时发生了错误，可能是ip地址错误或此端口已被占用，errno: " << errno);
			return false;
		}

		if (::listen(m_fd, SOMAXCONN) < 0)
		{
			_Close();
			cxLogerError("socket异步启动服务端失败，启动监听时发生了错误，errno: " << errno);
			return false;
		}

		m_bconnected = true;
		m_blisten = true;
		m_cb_onaccept = cb;

		if (!_BindEpoll(EPOLLIN | EPOLLET | EPOLLONESHOT))
		{
			_Close();
			cxLogerError("socket异步启动服务端失败，绑定到epoll模型失败");
			return false;
		}

		return true;
	}
	bool cxSocketTcp::_BindEpoll(int flag)
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(struct epoll_event));
		ev.events = flag;
		ev.data.fd = m_fd;
		if (m_bind_epoll)
		{
			if (epoll_ctl(g_epoll, EPOLL_CTL_MOD, m_fd, &ev) < 0)
			{
				return false;
			}
		}
		else
		{
			if (epoll_ctl(g_epoll, EPOLL_CTL_ADD, m_fd, &ev) < 0)
			{
				return false;
			}
			m_bind_epoll = true;
		}
		return true;
	}
	void cxSocketTcp::_UnBindEpoll()
	{
		if (m_bind_epoll)
		{
			epoll_ctl(g_epoll, EPOLL_CTL_DEL, m_fd, NULL);
			m_bind_epoll = false;
		}
	}
	void cxSocketTcp::_Close()
	{
		if (m_fd != -1)
		{
			_UnBindEpoll();

			// 必须先使用setsockopt后才能执行close，以防有数据还在发送缓冲区没有来得及发送
			struct linger tmp = { 0, 0 };
			setsockopt(m_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
			close(m_fd);
			m_fd = -1;

			//cxLogerEnable("socket[" << m_ip << ":" << m_port << "]连接断开");
			CBF_OnClose cb = m_cb_onclose;

			m_ip = "";
			m_port = 0;
			m_bind_epoll = false;
			m_bconnected = false;
			m_blisten = false;
			m_timeout = 0;
			m_tick = 0;
			m_cb_onaccept = NULL;
			m_cb_onrecv = NULL;
			m_cb_onclose = NULL;
			m_sem_connect.ReleaseOne();

			if (cb != NULL)
			{
				cx::cxThreadPool::RunThread(cb, shared_from_this());
			}
		}
	}
	bool cxSocketTcp::_Connect(const std::string& ip, int port)
	{
		if (m_bconnected)
		{
			return false;
		}

		std::string ipadd = GetHostByName(ip);

		m_ip = ipadd;
		m_port = port;

		// 创建ip端口参数
		struct sockaddr_in ServerAddress;
		memset((char*)&ServerAddress, 0, sizeof(ServerAddress));
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_addr.s_addr = inet_addr(ipadd.c_str());
		ServerAddress.sin_port = htons(port);
		int iret = connect(m_fd, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
		if (iret == 0)
		{
			m_bconnected = true;
		}
		m_sem_connect.ReleaseOne();
		return iret == 0;
	}
	void cxSocketTcp::_Thread_Connect(Ptr ptr, const std::string& ip, int port)
	{
		ptr->_Connect(ip, port);
	}

	cxSafeMap<SocketFD, cxSocketTcp::Ptr>							cxSocketTcp::g_sockets;
	SocketFD														cxSocketTcp::g_epoll = ERROR_SOCKET;
	bool															cxSocketTcp::g_init = false;
	std::mutex														cxSocketTcp::g_mutex;

	bool cxSocketTcp::GInit()
	{
		if (g_init) return true;
		std::unique_lock<std::mutex> Lock(g_mutex);
		if (g_init) return true;

		int threads = cxCommonFun::GetCpuCount() * 2;
		if (threads <= 0)
		{
			cxLogerError("tcp_socket底层服务初始化失败，获取cpu核心数量失败");
			return false;
		}

		// 创建epoll句柄
		g_epoll = epoll_create1(0);
		if (g_epoll <= 0)
		{
			cxLogerError("tcp_socket底层服务初始化失败，epoll模型创建失败");
			return false;
		}

		// 启动epoll工作线程
		for (int i = 0; i < threads; i++)
		{
			// 启动epoll监控服务线程
			std::thread td_service(_Thread_Service);
			td_service.detach();
		}

		// 启动超时监控线程
		std::thread td_timeout(_Thread_Service_TimeOut);
		td_timeout.detach();

		g_init = true;

		cxLogerEnable("tcp_socket底层服务初始化成功，工作线程数：" << threads);

		return true;
	}
	void cxSocketTcp::_Thread_Service()
	{
		struct epoll_event evlist;
		unsigned char recv_buf[RECVBUFSIZE];
		while (true)
		{
			// 监控epoll
			int ret = epoll_wait(g_epoll, &evlist, 1, -1);
			if (ret == 0)
			{
				continue;
			}

			if (ret < 0)
			{
				if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				{
					continue;
				}

				// 严重错误，epoll出错
				cxLogerError("系统错误，tcp_socket底层服务停止运行，epoll模型出错！errno = " << errno);
				return;
			}

			std::shared_ptr<cxSocketTcp> sock = NULL;
			sock = g_sockets.get(evlist.data.fd);
			if (sock == NULL)
			{
				continue;
			}

			if ((evlist.events & EPOLLIN) == EPOLLIN) // 可读事件
			{
				if (sock->m_blisten)
				{
					_Thread_Work_OnAccept(sock);
				}
				else
				{
					_Thread_Work_OnRecv(sock, recv_buf, RECVBUFSIZE);
				}
			}
		}
	}
	void cxSocketTcp::_Thread_Service_TimeOut()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			
			for (unsigned int i = 0; i < g_sockets.size(); i++)
			{
				Ptr ptr = g_sockets.get(i);
				if (ptr == NULL)
				{
					continue;
				}

				if (ptr->m_timeout > 0 && ptr->m_tick > 0)
				{
					unsigned long long tick = cxCommonFun::GetTickTime();
					if (tick - ptr->m_tick >= ptr->m_timeout)
					{
						cxLogerWarning("socket[" << ptr->m_fd << "][" << ptr->m_ip << ":" << ptr->m_port << "]接收数据超时");
						ptr->Close();
					}
				}
			}
		}
	}
	void cxSocketTcp::_Thread_Work_OnRecv(Ptr sock, void* buf, unsigned int buf_size)
	{
		//std::unique_lock<std::mutex> Lock(sock->m_mutex_recv);

		while (true)
		{
			int recv_len = recv(sock->m_fd, buf, buf_size, MSG_NOSIGNAL | MSG_DONTWAIT);
			sock->m_tick = cxCommonFun::GetTickTime();
			if (recv_len < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else if (errno == EAGAIN)
				{
					if (!sock->_BindEpoll(EPOLLIN | EPOLLET | EPOLLONESHOT))
					{
						sock->Close();
					}
				}
				else
				{
					sock->Close();
				}
			}
			else if (recv_len == 0) // 连接已关闭
			{
				sock->Close();
			}
			else
			{
				CBF_OnRecv cb = sock->m_cb_onrecv;
				if (cb != NULL)
				{
					cb(sock, buf, recv_len);
				}
			}
			return;
		}
	}
	void cxSocketTcp::_Thread_Work_OnAccept(Ptr sock)
	{
		while (true)
		{
			struct sockaddr_in cliaddr;
			memset(&cliaddr, 0, sizeof(cliaddr));
			unsigned int cliaddrlen1 = sizeof(cliaddr);
			SocketFD fd = accept(sock->m_fd, (sockaddr*)&cliaddr, &cliaddrlen1);
			if (fd < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else if (errno == EAGAIN || errno == EMFILE)
				{
					if (!sock->_BindEpoll(EPOLLIN | EPOLLET | EPOLLONESHOT))
					{
						sock->Close();
					}
					return;
				}
				else
				{
					sock->Close();
					return;
				}
			}
			
			Ptr client = cxSocketTcp::CreateSocket(fd);
			if (client == NULL)
			{
				continue;
			}

			client->m_ip = std::string(inet_ntoa(cliaddr.sin_addr));
			client->m_port = ntohs(cliaddr.sin_port);
			client->m_bconnected = true;
			client->m_tick = cx::cxCommonFun::GetTickTime();

			//cxLogerEnable("socket[" << client->m_ip << ":" << client->m_port << "]接入服务器");
			CBF_OnAccept cb = sock->m_cb_onaccept;
			if (cb != NULL)
			{
				cb(sock, client);
			}
		}
	}
}
