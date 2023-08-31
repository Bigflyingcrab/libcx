#pragma once
#include "cx.h"

class SysCommand : public cx::cxHttpController
{
public:
	void GET(cx::cxHttpRequest::Ptr request, cx::cxHttpResponse::Ptr response);
	void POST(const Json::Value& request, Json::Value& response);
};
	
class csHttpServer
{
private:
	static cx::cxHttpServer::Ptr		m_server;
	static std::mutex					m_mutex;
public:
	static bool IsRunning();
	static bool StartService(std::string ip, int port);
	static void StopService();
};