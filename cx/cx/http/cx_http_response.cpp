#include "cx_http_response.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxHttpResponse::cxHttpResponse(cxHttpStream::Ptr stream) : cxHttpEntity(stream)
	{
		m_header["Server"] = "libcx";
		m_header["Connection"] = "close";
	}
	cxHttpResponse::~cxHttpResponse()
	{

	}
	int cxHttpResponse::GetCode()
	{
		return m_code;
	}
	bool cxHttpResponse::ParseHeaderString(std::string header)
	{
		std::vector<std::string> lines;
		cxCommonFun::StringToLineGroup(header, lines);
		if (lines.size() <= 1)
		{
			return false;
		}

		std::vector<std::string> group;
		cx::cxCommonFun::StringToGroup(lines[0], " ", group);
		if (group.size() < 3 || group[0] == "" || group[1] == "" || group[2] == "")
		{
			return false;
		}

		m_version = group[0];
		m_code = cxCommonFun::StringToObject<int>(group[1]);
		
		for (unsigned int i = 1; i < lines.size(); i++)
		{
			std::vector<std::string> temp;
			cx::cxCommonFun::StringToGroup(lines[i], ": ", temp);
			if (temp.size() != 2)
			{
				continue;
			}

			m_header[temp[0]] = temp[1];
		}

		return true;
	}
	bool cxHttpResponse::Response(int code)
	{
		if (m_stream == NULL || !m_stream->IsConnected())
		{
			return false;
		}
		std::stringstream ss;
		ss << m_version << " " << code << " " << GetResponseCodeString(code) << "\r\n";
		auto it = m_header.begin();
		while (it != m_header.end())
		{
			if (it->first != "" && it->second != "")
			{
				ss << it->first << ": " << it->second << "\r\n";
			}
			it++;
		}
		ss << "\r\n";

		std::string str_header = ss.str();
		//cxLogerEnable("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]应答：" << str_header);
		bool bret = m_stream->m_socket->Send((void*)str_header.data(), str_header.size());
		if (!bret)
		{
			cxLogerError("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]发送应答数据失败");
		}
		return bret;
	}
	bool cxHttpResponse::Response(int code, Json::Value json)
	{
		if (json.isNull())
		{
			return Response(code);
		}
		else
		{
			m_header["Content-Type"] = "application/json";
			std::string str_json = json.toStyledString();
			m_header["Content-Length"] = std::to_string(str_json.size());
			if (!Response(code))
			{
				return false;
			}
			return SendBody(json);
		}
	}
	bool cxHttpResponse::Response(int code, const std::string& content_type, void* data, unsigned int data_size)
	{
		if (content_type == "" || data == NULL || data_size <= 0)
		{
			return Response(code);
		}
		else
		{
			m_header["Content-Type"] = content_type;
			m_header["Content-Length"] = std::to_string(data_size);
			if (!Response(code))
			{
				return false;
			}
			return SendBody(data, data_size);
		}
	}
}