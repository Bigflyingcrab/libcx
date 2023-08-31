#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <map>
#include <mutex>
#include "cx_socket_common.h"
#include "../thread/cx_thread.h"

namespace cx
{
	class cxSocketTcp : public std::enable_shared_from_this<cxSocketTcp>
	{
	public:
		using Ptr = std::shared_ptr<cxSocketTcp>;
		using CBF_OnAccept = std::function<void(Ptr server, Ptr sock)>;
		using CBF_OnRecv = std::function<void(Ptr sock, void* data, unsigned int size)>;
		using CBF_OnClose = std::function<void(Ptr sock)>;
	private:
		SocketFD									m_fd = ERROR_SOCKET;
		//bool										m_binit = false;
		bool										m_bind_epoll = false;
		bool										m_bconnected = false;
		bool										m_blisten = false;
		std::string									m_ip = "";
		int											m_port = 0;
		std::mutex									m_mutex;
		std::mutex									m_mutex_send;
		std::mutex									m_mutex_recv;
	private:
		unsigned long long							m_timeout = 0;
		unsigned long long							m_tick = 0;
		cxSemaphore									m_sem_connect;
	private:
		CBF_OnAccept								m_cb_onaccept = NULL;
		CBF_OnRecv									m_cb_onrecv = NULL;
		CBF_OnClose									m_cb_onclose = NULL;
	public:
		cxSocketTcp();
		virtual ~cxSocketTcp();
		static std::shared_ptr<cxSocketTcp> CreateSocket(SocketFD fd = ERROR_SOCKET);
		SocketFD GetFD();
		std::string GetIp();
		int GetPort();		
		bool IsConnected();
		void Close();
		void SetOnClose(CBF_OnClose cb);
		void SetTimeout(unsigned int time_out);
		bool Send(void* data, unsigned int data_size);
		bool Recv(void* buf, unsigned int buf_size, int& recv_size);
		bool BeginRecv(CBF_OnRecv cb);
		bool Connect(const std::string& ip, int port, unsigned int time_out = 0);
		bool BeginListen(const std::string& ip, int port, CBF_OnAccept cb);
	private:
		bool _BindEpoll(int flag);
		void _UnBindEpoll();
		void _Close();
		bool _Connect(const std::string& ip, int port);
		static void _Thread_Connect(Ptr ptr, const std::string& ip, int port);
	// -----------------------------------------------------------------------------------------------------------------------------------
	// 分割线，以下为内部处理
	// -----------------------------------------------------------------------------------------------------------------------------------
	private: // 全局信息
		//static std::shared_ptr<std::map<SocketFD, Ptr>>				g_sockets;
		static cxSafeMap<SocketFD, Ptr>				g_sockets;
		//static std::mutex							g_sockets_mutex;
		static SocketFD								g_epoll;
		static bool									g_init;
		static std::mutex							g_mutex;
	private: // 全局线程与方法
		static bool GInit();
		static void _Thread_Service();
		static void _Thread_Service_TimeOut();
		static void _Thread_Work_OnRecv(Ptr sock, void* buf, unsigned int buf_size);
		static void _Thread_Work_OnAccept(Ptr sock);
	};
}

// 创建socket的必要形式
// std::shared_ptr<cx::cxSocketTcp> ptr = cx::cxSocketTcp::CreateSocket();
