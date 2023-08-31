#pragma once
#include <iostream>
#include <memory>
#include <map>
#include "cx_http_entity.h"
#include "cx_http_response.h"

namespace cx
{
	class cxHttpRequest : public cxHttpEntity, public std::enable_shared_from_this<cxHttpRequest>
	{
	public:
		using Ptr = std::shared_ptr<cxHttpRequest>;
	private:
		cxHttp_Mode								m_mode = cxHttp_Mode::GET;
		std::string								m_url = "";
		std::map<std::string, std::string>		m_url_param;
	public:
		cxHttpRequest(cxHttpStream::Ptr stream = NULL);
		~cxHttpRequest();
		template<typename T> T GetUrlParam(std::string key)
		{
			std::stringstream ss;
			ss << m_url_param[key];
			T ret;
			ss >> ret;
			return ret;
		}
		std::string GetUrl();
		cxHttp_Mode GetMode();
		bool ParseHeaderString(std::string header);
		bool Request(cxHttp_Mode mode, std::string url, std::string content_type = "", long long content_length = 0);
		std::shared_ptr<cxHttpResponse> GetResponse();
	private:
		bool _ParseRequestUrl(std::string url, std::string& host, int& port);
	};
}
