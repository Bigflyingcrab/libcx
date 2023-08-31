#include "cx_http_stream.h"
#include "cx_http_server.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxHttpStream::cxHttpStream(cxSocketTcp::Ptr sock, SocketFD server_fd)
	{
		m_socket = sock;
		m_server_fd = server_fd;
	}
	cxHttpStream::~cxHttpStream()
	{

	}
	std::string cxHttpStream::GetIp()
	{
		if (m_socket == NULL)
		{
			return "";
		}
		return m_socket->GetIp();
	}
	int cxHttpStream::GetPort()
	{
		if (m_socket == NULL)
		{
			return 0;
		}
		return m_socket->GetPort();
	}
	SocketFD cxHttpStream::GetFD()
	{
		if (m_socket == NULL)
		{
			return -1;
		}
		return m_socket->GetFD();
	}
	SocketFD cxHttpStream::GetServerFD()
	{
		return m_server_fd;
	}
	bool cxHttpStream::IsConnected()
	{
		if (m_socket == NULL)
		{
			return false;
		}
		return m_socket->IsConnected();
	}
	void cxHttpStream::Close()
	{
		if (m_socket == NULL)
		{
			return;
		}		
		m_socket->Close();
	}
	void cxHttpStream::Complete()
	{
		m_header_string = "";
		m_recv_buf.clear();
	}
	int cxHttpStream::ParseHeader()
	{
		if (m_socket == NULL || !m_socket->IsConnected())
		{
			return -1;
		}

		if (m_recv_buf.size() <= 0)
		{
			return 1;
		}

		while (!m_recv_buf.empty())
		{
			std::shared_ptr<std::vector<unsigned char>> ptr = m_recv_buf.front();

			for (unsigned int i = 0; i < ptr->size() - 3; i++)
			{
				if ((*ptr)[i] == '\r' && (*ptr)[i + 1] == '\n' && (*ptr)[i + 2] == '\r' && (*ptr)[i + 3] == '\n')
				{
					m_header_string = std::string(ptr->begin(), ptr->begin() + i + 4);					
					if (ptr->size() == i + 4)
					{
						m_recv_buf.pop_front();
					}
					else
					{
						ptr->erase(ptr->begin(), ptr->begin() + i + 4);
					}
					return 0;
				}
			}

			if (ptr->size() >= 8192)
			{
				cxLogerError("http stream获取http header数据失败，header数据长度已超过系统限制");
				return -1;
			}

			if (m_recv_buf.size() > 1)
			{
				m_recv_buf.pop_front();
				std::shared_ptr<std::vector<unsigned char>> temp = m_recv_buf.front();
				temp->insert(temp->begin(), ptr->begin(), ptr->end());
				continue;
			}
			else
			{
				return 1;
			}
		}

		return 1;
	}
	bool cxHttpStream::WS_SendData(std::string data)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x81);
		if (data.size() < 126)
		{
			buf.push_back(data.size());
		}
		else if (data.size() <= 65535)
		{
			buf.push_back(0x7E);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)data.size());
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)data.size());
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}
		if (data.size() > 0)
		{
			buf.insert(buf.end(), data.begin(), data.end());
		}
		bool bret = m_socket->Send((void*)buf.data(), buf.size());
		//if (bret)
		//{
		//	cxLogerDebug("发送websocket信息：" << data);
		//}
		return bret;
	}
	bool cxHttpStream::WS_SendData(void* data, unsigned int data_size)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x82);
		if (data_size < 126)
		{
			buf.push_back(data_size);
		}
		else if (data_size <= 65535)
		{
			buf.push_back(0x7E);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)data_size);
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)data_size);
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}
		if (data_size > 0)
		{
			buf.insert(buf.end(), (unsigned char*)data, (unsigned char*)data + data_size);
		}
		return m_socket->Send((void*)buf.data(), buf.size());
	}
	bool cxHttpStream::WS_Ping()
	{
		unsigned char buf[2] = { 0x89, 0x00 };
		return m_socket->Send((void*)buf, sizeof(buf));
	}
	bool cxHttpStream::WS_Pong()
	{
		unsigned char buf[2] = { 0x8A, 0x00 };
		return m_socket->Send((void*)buf, sizeof(buf));
	}
}