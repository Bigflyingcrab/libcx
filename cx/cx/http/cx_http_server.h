#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <mutex>
#include "../thread/cx_thread.h"
#include "../socket/cx_socket.h"
#include "cx_http_request.h"
#include "cx_http_response.h"
#include "cx_http_controller.h"

namespace cx
{
	class cxHttpServer : public std::enable_shared_from_this<cxHttpServer>
	{
	public:
		using Ptr = std::shared_ptr<cxHttpServer>;	
	public:
		cxHttpServer();
		~cxHttpServer();
		bool SetWebPath(const std::string& path);
		bool AddController(const std::string& url, std::shared_ptr<cxWebController> controller);
		bool StartServer(const std::string& ip, int port);
		bool IsRunning();
		void CloseServer();
	private:
		cxSocketTcp::Ptr											m_socket = NULL;
		std::unordered_map<std::string, std::shared_ptr<cxWebController>>		m_user_controller;
		std::string													m_web_path = "";
		std::shared_ptr<cxHttpController>							m_web_controller;
		std::mutex													m_mutex;
	private:
		static std::map<SocketFD, cxHttpServer::Ptr>				g_servers;
		static std::mutex											g_servers_mutex;
		static cxSafeMap<SocketFD, std::shared_ptr<cxHttpStream>>	g_streams;
	private:
		static void OnAccept(cxSocketTcp::Ptr server, cxSocketTcp::Ptr sock);
		static void OnRecv(cxSocketTcp::Ptr sock, void* data, unsigned int size);
		static void DoHttpController(std::shared_ptr<cxWebController> controller, cxHttpRequest::Ptr request);
		static void DoWebSocketController(std::shared_ptr<cxWebController> controller, cxHttpRequest::Ptr request);
		static void ParseWebSocketData(cxHttpStream::Ptr stream);
		static void DoWebSocketController_Message(
			cxHttpStream::Ptr stream, 
			int opcode, void* data, unsigned int data_size);
		static void OnClose(cxSocketTcp::Ptr sock);
	};
}
