#pragma once
#include "cx.h"

typedef struct csClientData
{
	std::string					token = "";
	std::string					command = "";
	std::vector<char>			recv_buf;
	cx::cxSemaphore				sem;
	int							bret = -1;
	std::vector<std::string>	lines;
}csClientData;

class SystemCommandProxy
{
private:
	static bool						g_init;
	static std::mutex				g_init_mutex;
	static std::string				g_ip;
	static int						g_port;
	static std::map<std::string, std::shared_ptr<csClientData>>	g_data_list;
	static std::mutex				g_data_list_mutex;
	static std::map<cx::SocketFD, std::string>			g_sockets;
public:
	static void InitConnectInfo(const std::string& ip, int port);
	static int Command(const std::string& command, std::vector<std::string>& lines, const std::string& su_pass, unsigned int time_out);
	static void OnRecv(cx::cxSocketTcp::Ptr sock, void* data, unsigned int size);
	static void OnClose(cx::cxSocketTcp::Ptr sock);
};