#include "cx_command_proxy.h"
#include <mutex>
#include "../socket/cx_socket.h"
#include "../loger/cx_loger.h"
#include "../http/cx_http.h"

namespace cx
{
	typedef struct csClientData
	{
		std::string					token = "";
		std::string					command = "";
		std::vector<char>			recv_buf;
		cx::cxSemaphore				sem;
		int							bret = -1;
		std::vector<std::string>	lines;
	}csClientData;

	bool													g_init = false;
	std::mutex												g_init_mutex;
	std::string												g_ip = "";
	int														g_port = 0;
	std::map<std::string, std::shared_ptr<csClientData>>	g_data_list;
	std::mutex												g_data_list_mutex;
	std::map<cx::SocketFD, std::string>						g_sockets;

	void cxCommandProxy::InitConnectInfo(const std::string& ip, int port)
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

		cxLogerEnable("系统命令代理初始化成功，ip: " << ip << ", port: " << port);
	}

	int cxCommandProxy::Command(const std::string& command, unsigned int time_out)
	{
		std::vector<std::string> lines;
		return Command(command, lines, time_out);
	}
	int cxCommandProxy::Command(const std::string& command, std::vector<std::string>& lines, unsigned int time_out)
	{
		if (!g_init || !g_init || g_ip == "" || g_port <= 0)
		{
			cxLogerError("系统命令代理调用失败，未初始化连接信息");
			return -1;
		}

		unsigned long long tick = cxCommonFun::GetTickTime();
		Json::Value json_req;
		json_req["command"] = command;
		std::string str_req = json_req.toStyledString();

		std::stringstream url;
		url << "http://" << g_ip << ":" << g_port << "/api/SysCommand";
		cxHttpRequest::Ptr request = std::make_shared<cxHttpRequest>();
		if (!request->Request(cxHttp_Mode::POST, url.str(), "application/json", str_req.size()))
		{
			if (request->Stream() != NULL)
			{
				request->Stream()->Close();
			}
			cxLogerError("系统命令代理调用失败，请求失败");
			return -1;
		}
		if (!request->SendBody((void*)str_req.data(), str_req.size()))
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，发送body失败");
			return -1;
		}

		cxHttpResponse::Ptr response = request->GetResponse();
		if (response == NULL)
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，获取应答失败");
			return -1;
		}
		if (response->GetCode() != 200)
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，应答错误");
			return -1;
		}

		std::shared_ptr<std::vector<char>> body;
		if (!response->ReadBodyToEnd(body) || body == NULL || body->size() <= 0)
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，获取body失败");
			return -1;
		}

		Json::Value json_res;
		Json::Reader read;
		std::string str_res(body->begin(), body->end());

		if (!read.parse(str_res, json_res))
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，解析应答json失败");
			return -1;
		}

		int ret = JSON_INT(json_res, "ret", -1);
		std::string desc = JSON_STR(json_res, "desc", "");
		if (ret == -1)
		{
			request->Stream()->Close();
			cxLogerError("系统命令代理调用失败，服务器应答了错误，ret = " << ret << ", desc = " << desc);
			return -1;
		}

		if (ret == -2)
		{
			request->Stream()->Close();
			return 0;
		}

		if (!json_res["lines"].isNull() && json_res["lines"].isArray())
		{
			for (int i = 0; i < json_res["lines"].size(); i++)
			{
				if (!json_res["lines"][i].isNull() && json_res["lines"][i].isString())
				{
					lines.push_back(json_res["lines"][i].asString());
				}
			}
		}

		request->Stream()->Close();
		return 1;
	}
}