#pragma once
#include "cx.h"
#include "cs_client.h"

class csServerClient : public std::enable_shared_from_this<csServerClient>
{
public:
	cx::cxSocketTcp::Ptr					sock;
	std::vector<unsigned char>				recv_buf;
public:
	csServerClient(cx::cxSocketTcp::Ptr socket);
	~csServerClient();
	void DoRecvData(void* data, unsigned int size);
	static void Thread_DoWork(std::shared_ptr<csServerClient> ptr, std::string token, std::string command);
};

class csServer : public std::enable_shared_from_this<csServer>
{
private:
	static std::map<cx::SocketFD, std::shared_ptr<csServerClient>>			m_clients;
	static std::mutex								m_clients_mutex;
	static cx::cxSocketTcp::Ptr						m_socket;
public:
	static bool StartService(std::string ip, int port);
	static void StopService();
	static void OnAccept(cx::cxSocketTcp::Ptr server, cx::cxSocketTcp::Ptr sock);
	static void OnRecv(cx::cxSocketTcp::Ptr sock, void* data, unsigned int size);
	static void OnClose_Server(cx::cxSocketTcp::Ptr sock);
	static void OnClose_Client(cx::cxSocketTcp::Ptr sock);
};