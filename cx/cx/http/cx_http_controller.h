#pragma once
#include <iostream>
#include <memory>
#include "../json/json.h"
#include "cx_http_request.h"
#include "cx_http_response.h"

namespace cx
{
	class cxWebController
	{
	protected:
		std::string							m_path = "";
		bool								m_bws = false;
		bool								m_bcross = true;
	public:
		cxWebController();
		virtual ~cxWebController();
		std::string GetPath();
		bool IsWebSocket();
		bool GetCross();
		virtual void GET(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void HEAD(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void POST(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void POST(const Json::Value& request, Json::Value& response);
		virtual void OPTIONS(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void PUT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void DELETE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void TRACE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void CONNECT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void WS_OnAccept(cxHttpStream::Ptr stream);
		virtual void WS_OnRecvBin(cxHttpStream::Ptr stream, void* data, unsigned int size);
		virtual void WS_OnRecvStr(cxHttpStream::Ptr stream, const std::string& data);
		virtual void WS_OnPing(cxHttpStream::Ptr stream);
		virtual void WS_OnPong(cxHttpStream::Ptr stream);
		virtual void WS_OnClose(cxHttpStream::Ptr stream);
	private:
		bool ParseRequestFile(cxHttpRequest::Ptr request, std::string& file_path, std::string& file_ext);
		void _GET_HEAD_(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response, bool bget);
		std::string GetContentType(std::string ext);
	};
	class cxHttpController : public cxWebController
	{
	public:
		cxHttpController();
		cxHttpController(std::string path);
		cxHttpController(bool bcross);
		cxHttpController(bool bcross, std::string path);
		virtual ~cxHttpController();
		std::string GetPath();
		bool IsWebSocket();
		virtual void GET(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void HEAD(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void POST(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void POST(const Json::Value& request, Json::Value& response);
		virtual void OPTIONS(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void PUT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void DELETE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void TRACE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
		virtual void CONNECT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response);
	};

	class cxWebSocketController : public cxWebController
	{
	public:
		cxWebSocketController();
		cxWebSocketController(bool bcross);
		virtual ~cxWebSocketController();
		virtual void WS_OnAccept(cxHttpStream::Ptr stream);
		virtual void WS_OnRecvBin(cxHttpStream::Ptr stream, void* data, unsigned int size);
		virtual void WS_OnRecvStr(cxHttpStream::Ptr stream, const std::string& data);
		virtual void WS_OnPing(cxHttpStream::Ptr stream);
		virtual void WS_OnPong(cxHttpStream::Ptr stream);
		virtual void WS_OnClose(cxHttpStream::Ptr stream);
	};
}
