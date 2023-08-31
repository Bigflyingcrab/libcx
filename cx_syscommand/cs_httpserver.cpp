#include "cs_httpserver.h"

void SysCommand::GET(cx::cxHttpRequest::Ptr request, cx::cxHttpResponse::Ptr response)
{
	response->Response(200);
}
void SysCommand::POST(const Json::Value& request, Json::Value& response)
{
	std::string command = JSON_STR(request, "command", "");
	if (command == "")
	{
		response["ret"] = -1;
		response["desc"] = "命令为空";
		return;
	}

	std::vector<std::string> lines;
	if (!cx::cxCommonFun::SystemCommand(command, lines))
	{
		response["ret"] = -2;
		response["desc"] = "调用命令失败";
		return;
	}

	response["ret"] = 0;
	response["lines"];
	for (unsigned int i = 0; i < lines.size(); i++)
	{
		response["lines"].append(lines[i]);
	}
}

cx::cxHttpServer::Ptr		csHttpServer::m_server = NULL;
std::mutex					csHttpServer::m_mutex;

bool csHttpServer::IsRunning()
{
	std::unique_lock<std::mutex> Lock(m_mutex);
	return m_server != NULL && m_server->IsRunning();
}
bool csHttpServer::StartService(std::string ip, int port)
{
	std::unique_lock<std::mutex> Lock(m_mutex);
	if (m_server != NULL)
	{
		return false;
	}

	m_server = std::make_shared<cx::cxHttpServer>();
	m_server->AddController("/api/SysCommand", std::make_shared<SysCommand>());
	if (!m_server->StartServer(ip, port))
	{
		m_server->CloseServer();
		m_server = NULL;
		return false;
	}

	return true;
}
void csHttpServer::StopService()
{
	std::unique_lock<std::mutex> Lock(m_mutex);
	if (m_server != NULL)
	{
		m_server->CloseServer();
		m_server = NULL;
	}
}
