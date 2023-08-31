#include "cs_server.h"

extern std::shared_ptr<cx::cxSemaphore> GSEM;

csServerClient::csServerClient(cx::cxSocketTcp::Ptr socket)
{
	sock = socket;
}
csServerClient::~csServerClient()
{
	if (sock != NULL)
	{
		sock->Close();
	}
}
void csServerClient::DoRecvData(void* data, unsigned int size)
{
	if (data != NULL && size > 0)
	{
		recv_buf.insert(recv_buf.end(), (unsigned char*)data, (unsigned char*)data + size);
	}

	if (recv_buf.size() < 4)
	{
		return;
	}

	unsigned int len = cx::Net2Host32(recv_buf.data());

	if (recv_buf.size() < len + 4)
	{
		return;
	}

	if (len == 0)
	{
		recv_buf.erase(recv_buf.begin(), recv_buf.begin() + 4);
		return;
	}

	if (len >= 8192)
	{
		sock->Close();
		return;
	}

	std::string str_req((char*)recv_buf.data() + 4, (char*)recv_buf.data() + 4 + len);
	recv_buf.erase(recv_buf.begin(), recv_buf.begin() + 4 + len);

	Json::Value json_req;
	Json::Reader read;

	if (read.parse(str_req, json_req))
	{
		if (!json_req["token"].isNull() && !json_req["command"].isNull() &&
			json_req["token"].isString() && json_req["command"].isString())
		{
			cx::cxThreadPool::RunThread(Thread_DoWork, shared_from_this(), json_req["token"].asString(), json_req["command"].asString());
		}
	}

	DoRecvData(NULL, 0);
}
void csServerClient::Thread_DoWork(std::shared_ptr<csServerClient> ptr, std::string token, std::string command)
{
	if (ptr == NULL || ptr->sock == NULL || !ptr->sock->IsConnected() || token == "" || command == "")
	{
		return;
	}

	std::vector<std::string> lines;
	bool ret = cx::cxCommonFun::SystemCommand(command, lines);

	Json::Value json_res;
	json_res["token"] = token;
	json_res["command"] = command;
	json_res["ret"] = ret;
	json_res["lines"];
	for (unsigned int i = 0; i < lines.size(); i++)
	{
		json_res["lines"].append(lines[i]);
	}

	std::string str_res = json_res.toStyledString();
	//cxLogerEnable("Command请求[" << ptr->sock->GetIp() << ":" << ptr->sock->GetPort() << "]: " << str_res);

	unsigned int len = str_res.size();

	char buf_len[4];
	cx::Host2Net((unsigned char*)buf_len, len);

	std::vector<unsigned char> send_data;
	send_data.insert(send_data.end(), (unsigned char*)buf_len, (unsigned char*)buf_len + 4);
	send_data.insert(send_data.end(), (unsigned char*)str_res.data(), (unsigned char*)str_res.data() + str_res.size());

	ptr->sock->Send((void*)send_data.data(), send_data.size());
}

std::map<cx::SocketFD, std::shared_ptr<csServerClient>>			csServer::m_clients;
std::mutex									csServer::m_clients_mutex;
cx::cxSocketTcp::Ptr						csServer::m_socket = NULL;

bool csServer::StartService(std::string ip, int port)
{
	if (m_socket != NULL && m_socket->IsConnected())
	{
		return false;
	}

	m_socket = cx::cxSocketTcp::CreateSocket();
	if (m_socket == NULL)
	{
		return false;
	}

	if (!m_socket->BeginListen(ip, port, OnAccept))
	{
		return false;
	}

	m_socket->SetOnClose(OnClose_Server);

	return true;
}
void csServer::StopService()
{
	if (m_socket != NULL)
	{
		m_socket->Close();
	}
}
void csServer::OnAccept(cx::cxSocketTcp::Ptr server, cx::cxSocketTcp::Ptr sock)
{
	m_clients_mutex.lock();
	m_clients[sock->GetFD()] = std::make_shared<csServerClient>(sock);
	m_clients_mutex.unlock();
	sock->SetOnClose(OnClose_Client);
	sock->BeginRecv(OnRecv);
}
void csServer::OnRecv(cx::cxSocketTcp::Ptr sock, void* data, unsigned int size)
{
	if (sock == NULL)
	{
		return;
	}
	std::shared_ptr<csServerClient> ptr = m_clients[sock->GetFD()];
	if (ptr != NULL)
	{
		ptr->DoRecvData(data, size);
		if (sock->IsConnected())
		{
			sock->BeginRecv(OnRecv);
		}
	}
	else
	{
		sock->Close();
	}
}
void csServer::OnClose_Server(cx::cxSocketTcp::Ptr sock)
{
	cxLogerEnable("服务器socket已关闭");
	GSEM->ReleaseOne();
}
void csServer::OnClose_Client(cx::cxSocketTcp::Ptr sock)
{
	m_clients[sock->GetFD()] = NULL;
}