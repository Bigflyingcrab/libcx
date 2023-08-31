#include "cs_client.h"

bool						SystemCommandProxy::g_init = false;
std::mutex					SystemCommandProxy::g_init_mutex;
std::string					SystemCommandProxy::g_ip = "";
int							SystemCommandProxy::g_port = 0;
std::map<std::string, std::shared_ptr<csClientData>>	SystemCommandProxy::g_data_list;
std::mutex					SystemCommandProxy::g_data_list_mutex;
std::map<cx::SocketFD, std::string>			SystemCommandProxy::g_sockets;

void SystemCommandProxy::InitConnectInfo(const std::string& ip, int port)
{
	if (g_init) return;
	std::unique_lock<std::mutex> Lock(g_init_mutex);
	if (g_init) return;
	g_ip = ip;
	g_port = port;
	if (g_ip != "" && g_port > 0)
	{
		g_init = true;
	}
}
int SystemCommandProxy::Command(const std::string& command, std::vector<std::string>& lines, const std::string& su_pass, unsigned int time_out)
{
	if (!g_init)
	{
		cxLogerError("系统命令代理调用失败，未初始化连接信息");
		return -1;
	}

	cx::cxSocketTcp::Ptr socket = cx::cxSocketTcp::CreateSocket();
	if (socket == NULL)
	{
		cxLogerError("系统命令代理调用失败，socket创建失败");
		return -1;
	}

	if (!socket->Connect(g_ip, g_port, 3000))
	{
		cxLogerError("系统命令代理调用失败，连接代理服务器失败");
		return -1;
	}

	std::string str_token = cx::cxCommonFun::CreateToken();
	std::string str_command = "";
	if (su_pass == "")
	{
		str_command = command;
	}
	else
	{
		str_command = "echo " + su_pass + " | sudo -S " + command;
	}

	Json::Value json_req;
	json_req["token"] = str_token;
	json_req["command"] = str_command;

	std::string str_req = json_req.toStyledString();
	unsigned int len = str_req.size();
	unsigned char len_buf[4];
	cx::Host2Net(len_buf, len);

	std::vector<unsigned char> send_buf(len_buf, len_buf + 4);
	send_buf.insert(send_buf.end(), str_req.data(), str_req.data() + str_req.size());
	
	socket->SetOnClose(OnClose);
	socket->BeginRecv(OnRecv);
	std::shared_ptr<csClientData> cd = std::make_shared<csClientData>();
	cd->token = str_token;
	cd->command = str_command;
	g_data_list_mutex.lock();
	g_data_list[str_token] = cd;
	g_data_list_mutex.unlock();
	g_sockets[socket->GetFD()] = str_token;
	if (!socket->Send((void*)send_buf.data(), send_buf.size()))
	{
		cxLogerError("系统命令代理调用失败，将请求发送至代理服务器失败");
		return -1;
	}

	if (!cd->sem.WaitOne(time_out))
	{
		cxLogerError("系统命令代理调用失败，等待代理服务器应答超时");
		return -1;
	}

	if (cd->bret >= 0)
	{
		for (unsigned int i = 0; i < cd->lines.size(); i++)
		{
			lines.push_back(cd->lines[i]);
		}
	}

	return cd->bret;
}
void SystemCommandProxy::OnRecv(cx::cxSocketTcp::Ptr sock, void* data, unsigned int size)
{
	if (sock == NULL || data == NULL || size <= 0)
	{
		return;
	}
	std::string token = g_sockets[sock->GetFD()];
	if (token == "")
	{
		return;
	}

	std::shared_ptr<csClientData> cd = g_data_list[token];
	if (cd == NULL)
	{
		return;
	}

	cd->recv_buf.insert(cd->recv_buf.end(), (char*)data, (char*)data + size);
	if (cd->recv_buf.size() <= 4)
	{
		sock->BeginRecv(OnRecv);
		return;
	}

	unsigned int len = cx::Net2Host32((unsigned char*)cd->recv_buf.data());
	if (cd->recv_buf.size() < 4 + len)
	{
		sock->BeginRecv(OnRecv);
		return;
	}

	std::string str_res(cd->recv_buf.data() + 4, cd->recv_buf.data() + 4 + len);
	Json::Value json_res;
	Json::Reader read;
	if (!read.parse(str_res, json_res) || json_res["ret"].isNull())
	{
		cd->bret = -1;
		cd->sem.ReleaseOne();
		cxLogerError("解析代理服务器数据失败");
		return;
	}

	cd->bret = json_res["ret"].asBool() ? 1 : 0;
	if (!json_res["lines"].isNull())
	{
		for (int i = 0; i < json_res["lines"].size(); i++)
		{
			cd->lines.push_back(json_res["lines"][i].asString());
		}
	}
	cd->sem.ReleaseOne();
}
void SystemCommandProxy::OnClose(cx::cxSocketTcp::Ptr sock)
{
	if (sock == NULL)
	{
		return;
	}
	std::string token = g_sockets[sock->GetFD()];
	if (token == "")
	{
		return;
	}

	std::shared_ptr<csClientData> cd = g_data_list[token];
	if (cd == NULL)
	{
		return;
	}

	cd->sem.ReleaseOne();
	g_sockets[sock->GetFD()] = "";
	g_data_list[token] = NULL;
}
