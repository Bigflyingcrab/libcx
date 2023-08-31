#include "cx_http_request.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxHttpRequest::cxHttpRequest(cxHttpStream::Ptr stream) : cxHttpEntity(stream)
	{
		//m_stream = stream;
	}
	cxHttpRequest::~cxHttpRequest()
	{

	}
	std::string cxHttpRequest::GetUrl()
	{
		return m_url;
	}
	cxHttp_Mode cxHttpRequest::GetMode()
	{
		return m_mode;
	}
	bool cxHttpRequest::ParseHeaderString(std::string header)
	{
		std::vector<std::string> lines;
		cxCommonFun::StringToLineGroup(header, lines);
		if (lines.size() <= 1)
		{
			return false;
		}

		std::vector<std::string> group;
		cx::cxCommonFun::StringToGroup(lines[0], " ", group);
		if (group.size() != 3)
		{
			return false;
		}

		if (group[0] == "GET") m_mode = cxHttp_Mode::GET;
		else if (group[0] == "HEAD") m_mode = cxHttp_Mode::HEAD;
		else if (group[0] == "POST") m_mode = cxHttp_Mode::POST;
		else if (group[0] == "OPTIONS") m_mode = cxHttp_Mode::OPTIONS;
		else if (group[0] == "PUT") m_mode = cxHttp_Mode::PUT;
		else if (group[0] == "DELETE") m_mode = cxHttp_Mode::DELETE;
		else if (group[0] == "TRACE") m_mode = cxHttp_Mode::TRACE;
		else if (group[0] == "CONNECT") m_mode = cxHttp_Mode::CONNECT;
		else return false;

		std::string key = "";
		std::string value = "";
		int index = group[1].find("?");
		if (index > 0)
		{
			m_url = group[1].substr(0, index);
			group[1] = group[1].substr(index + 1);			
			bool bkey = true;
			for (unsigned int i = 0; i < group[1].size(); i++)
			{
				if (group[1][i] == '=')
				{
					bkey = false;
					continue;
				}
				else if (group[1][i] == '&')
				{
					if (key != "" && value != "")
					{
						m_url_param[key] = value;
					}
					key = "";
					value = "";
				}
				else
				{
					if (bkey)
					{
						key += group[1][i];
					}
					else
					{
						value += group[1][i];
					}
				}
			}

			if (key != "" && value != "")
			{
				m_url_param[key] = value;
			}
		}
		else
		{
			m_url = group[1];
		}

		m_version = group[2];

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

		//cxLogerEnable("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]请求：" << header);
		return true;
	}
	bool cxHttpRequest::Request(cxHttp_Mode mode, std::string url, std::string content_type, long long content_length)
	{
		m_mode = mode;
		std::string host = "";
		int port = 0;
		if (!_ParseRequestUrl(url, host, port))
		{
			return false;
		}
		if (content_type != "")
		{
			m_header["Content-Type"] = content_type;
			if (content_length > 0)
			{
				m_header["Content-Length"] = std::to_string(content_length);
			}
		}

		if (m_stream != NULL)
		{
			m_stream->Close();			
		}

		cxSocketTcp::Ptr socket = cxSocketTcp::CreateSocket();
		if (socket == NULL)
		{
			return false;
		}

		m_stream = std::make_shared<cxHttpStream>(socket, -1);
		if (!m_stream->m_socket->Connect(host, port, 20000))
		{
			return false;
		}

		std::stringstream ss;
		switch (m_mode)
		{
		case cxHttp_Mode::GET: ss << "GET "; break;
		case cxHttp_Mode::POST: ss << "POST "; break;
		case cxHttp_Mode::HEAD: ss << "HEAD "; break;
		case cxHttp_Mode::OPTIONS: ss << "OPTIONS "; break;
		case cxHttp_Mode::PUT: ss << "PUT "; break;
		case cxHttp_Mode::DELETE: ss << "DELETE "; break;
		case cxHttp_Mode::TRACE: ss << "TRACE "; break;
		case cxHttp_Mode::CONNECT: ss << "CONNECT "; break;
		default: return false;
		}

		ss << m_url;
		if (m_url_param.size() > 0)
		{
			ss << "?";
			auto it = m_url_param.begin();
			while (it != m_url_param.end())
			{
				if (it->first != "" && it->second != "")
				{
					ss << it->first << "=" << it->second;
				}
				it++;
			}
		}
		ss << " " << m_version << "\r\n";

		ss << "Host: " << host << ":" << port << "\r\n";
		ss << "Accept: */*\r\n";

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

		std::string str = ss.str();

		m_stream->m_socket->SetTimeout(20000);

		return m_stream->m_socket->Send((void*)str.data(), str.size());
	}
	std::shared_ptr<cxHttpResponse> cxHttpRequest::GetResponse()
	{
		while (true)
		{
			unsigned char buf[8192];
			int len = 0;
			if (!m_stream->m_socket->Recv(buf, sizeof(buf), len))
			{
				return NULL;
			}

			m_stream->m_recv_buf.push_back(std::make_shared<std::vector<unsigned char>>(buf, buf + len));

			int iret = m_stream->ParseHeader();
			if (iret < 0)
			{
				return NULL;
			}
			else if (iret == 0)
			{
				break;
			}
		}
		std::shared_ptr<cxHttpResponse> response = std::make_shared<cxHttpResponse>(m_stream);
		if (!response->ParseHeaderString(m_stream->m_header_string))
		{
			return NULL;
		}
		return response;
	}
	bool cxHttpRequest::_ParseRequestUrl(std::string url, std::string& host, int& port)
	{
		int index = url.find("http://");
		if (index != 0)
		{
			return false;
		}
		url = url.substr(7);
		index = url.find("/");
		if (index > 0)
		{
			m_url = url.substr(index);
			url = url.substr(0, index);
		}
		else
		{
			m_url = "/";
		}

		std::vector<std::string> group;
		cxCommonFun::StringToGroup(url, ":", group);
		if (group.size() <= 0)
		{
			return false;
		}
		host = group[0];
		if (group.size() > 1)
		{
			port = cxCommonFun::StringToObject<int>(group[1]);
		}
		else
		{
			port = 80;
		}

		std::string param = "";
		index = m_url.find("?");
		if (index > 0)
		{
			param = m_url.substr(index + 1);
			m_url = m_url.substr(0, index);
		}

		if (param != "")
		{
			std::string key = "";
			std::string value = "";
			bool bkey = true;

			for (unsigned int i = 0; i < param.size(); i++)
			{
				if (param[i] == '&')
				{
					if (key != "" && value != "")
					{
						m_url_param[key] = value;
					}
					key = "";
					value = "";
					bkey = true;
					continue;
				}
				else if (param[i] == '=')
				{
					bkey = false;
					continue;
				}
				else
				{
					if (bkey)
					{
						key += param[i];
					}
					else
					{
						value += param[i];
					}
				}
			}
			if (key != "" && value != "")
			{
				m_url_param[key] = value;
			}
		}

		return true;
	}
}
