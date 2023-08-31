#pragma once
#include "cx_http_entity.h"

namespace cx
{
	class cxHttpResponse : public cxHttpEntity, public std::enable_shared_from_this<cxHttpResponse>
	{
	public:
		using Ptr = std::shared_ptr<cxHttpResponse>;
	private:
		int										m_code = 0;
	public:
		cxHttpResponse(cxHttpStream::Ptr stream = NULL);
		~cxHttpResponse();
		int GetCode();
		bool ParseHeaderString(std::string header);
		bool Response(int code);
		bool Response(int code, Json::Value json);
		bool Response(int code, const std::string& content_type, void* data, unsigned int data_size);
	};
}
