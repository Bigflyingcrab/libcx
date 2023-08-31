#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <fstream>
#include "cx_http_stream.h"
#include "../loger/cx_loger.h"

namespace cx
{
	enum class cxHttp_Mode
	{
		GET,
		HEAD,
		POST,
		OPTIONS,
		PUT,
		DELETE,
		TRACE,
		CONNECT
	};

	class cxHttpEntity : public std::enable_shared_from_this<cxHttpEntity>
	{
	protected:
		cxHttpStream::Ptr								m_stream;
		std::map<std::string, std::string>				m_header;
		std::string										m_version = "HTTP/1.1";
		long long										m_readbody_len = 0;
	public:
		cxHttpEntity(cxHttpStream::Ptr stream)
		{
			m_stream = stream;
		}
		virtual ~cxHttpEntity()
		{

		}
		cxHttpStream::Ptr Stream()
		{
			return m_stream;
		}
		void SetHeader(std::string key, std::string value)
		{
			m_header[key] = value;
		}
		void SetHeader(std::string key, long long value)
		{
			m_header[key] = std::to_string(value);
		}
		template<typename T> T GetHeader(std::string key)
		{
			std::stringstream ss;
			ss << m_header[key];
			T ret;
			ss >> ret;
			return ret;
		}
		int ReadBody(void* buf, unsigned int buf_size, int& len)
		{
			long long content_len = cxCommonFun::StringToObject<long long>(m_header["Content-Length"]);
			if (m_readbody_len >= content_len)
			{
				return 0;
			}
			if (m_stream == NULL || m_stream->m_socket == NULL || !m_stream->m_socket->IsConnected())
			{
				return -1;
			}
			while (!m_stream->m_recv_buf.empty())
			{
				std::shared_ptr<std::vector<unsigned char>> temp = m_stream->m_recv_buf.front();
				if (temp != NULL && !temp->empty())
				{
					memcpy(buf, temp->data(), temp->size());
					len = (int)temp->size();
					m_readbody_len += len;
					m_stream->m_recv_buf.pop_front();
					if (m_readbody_len >= content_len)
					{
						return 0;
					}
					else
					{
						return 1;
					}
				}
				m_stream->m_recv_buf.pop_front();
			}
			buf_size = cxCommonFun::MIN<unsigned int>(buf_size, (unsigned int)(content_len - m_readbody_len));
			if (!m_stream->m_socket->Recv(buf, buf_size, len))
			{
				return -1;
			}
			m_readbody_len += len;
			if (m_readbody_len >= content_len)
			{
				return 0;
			}
			return 1;
		}
		bool ReadBodyToEnd(std::shared_ptr<std::vector<char>>& buf)
		{
			std::shared_ptr<std::vector<char>> temp = std::make_shared<std::vector<char>>();
			char cbuf[16384];
			while (true)
			{
				int recv_len = 0;
				int iret = ReadBody((void*)cbuf, sizeof(cbuf), recv_len);
				if (iret < 0)
				{
					return false;
				}
				if (recv_len > 0)
				{
					temp->insert(temp->end(), cbuf, cbuf + recv_len);
				}
				if (iret == 0)
				{
					buf = temp;
					return true;
				}
			}
		}
		bool SendBody(void* data, unsigned int data_size)
		{
			if (data == NULL || data_size <= 0)
			{
				return true;
			}

			bool bret = m_stream->m_socket->Send(data, data_size);

			if (!bret)
			{
				cxLogerError("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]发送body数据失败");
			}
			return bret;
		}
		bool SendBody(Json::Value json)
		{
			if (json.isNull())
			{
				return true;
			}
			std::string str = json.toStyledString();
			bool bret = SendBody((void*)str.data(), (unsigned int)str.size());

			if (!bret)
			{
				cxLogerError("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]发送body数据失败");
			}
			return bret;
		}
		bool SendBody(std::ifstream& file, long long file_size)
		{
			if (!file.is_open())
			{
				return false;
			}

			if (file_size <= 0)
			{
				return true;
			}

			while (file && file_size > 0)
			{
				char buf[16384];
				std::streampos pos = file.tellg();
				long long size = cxCommonFun::MIN<long long>(sizeof(buf), file_size);
				file.read(buf, size);
				long long read_len = file.tellg() - pos;
				if (read_len > 0)
				{
					file_size -= read_len;
					if (!m_stream->m_socket->Send((void*)buf, (unsigned int)read_len))
					{
						//cxLogerError("http客户端[" << m_stream->m_socket->GetIp() << ":" << m_stream->m_socket->GetPort() << "]发送body数据失败");
						return false;
					}
				}
				else
				{
					return false;
				}
			}

			return true;
		}
		void ParseRangeInfo(std::string range, long long file_size, long long& res_size, long long& pos_start, long long& pos_end)
		{
			int index = range.find("bytes=");
			if (index != 0)
			{
				res_size = file_size;
				pos_start = 0;
				pos_end = file_size - 1;
				return;
			}

			range = range.substr(6);

			std::string str_start = "";
			std::string str_end = "";
			bool bstart = true;
			for (unsigned int i = 0; i < range.length(); i++)
			{
				if (range[i] == '-')
				{
					bstart = false;
					continue;
				}
				if (range[i] < '0' || range[i] > '9')
				{
					break;
				}
				if (bstart)
				{
					str_start += range[i];
				}
				else
				{
					str_end += range[i];
				}
			}

			long long istart = cxCommonFun::StringToObject<long long>(str_start);
			long long iend = cxCommonFun::StringToObject<long long>(str_end);
			if (str_start != "" && str_end != "") // 100-200
			{
				pos_start = istart;
				pos_end = iend;
			}
			else if (str_start != "" && str_end == "") // 100-
			{
				pos_start = istart;
				pos_end = file_size - 1;
			}
			else if (str_start == "" && str_end != "") // -100
			{
				pos_end = file_size - 1;
				pos_start = file_size - iend;
			}

			res_size = pos_end - pos_start + 1;
		}
		virtual bool ParseHeaderString(std::string header) = 0;
	};

	inline std::string GetResponseCodeString(int code)
	{
		std::string ret = "";

		switch (code)
		{
		case 100: ret = "Continue"; break;
		case 101: ret = "Switching Protocols"; break;
		case 200: ret = "OK"; break;
		case 201: ret = "Created"; break;
		case 202: ret = "Accepted"; break;
		case 203: ret = "Non Authoritative Information"; break;
		case 204: ret = "No Content"; break;
		case 205: ret = "Reset Content"; break;
		case 206: ret = "Partial Content"; break;
		case 300: ret = "Multiple Choices"; break;
		case 301: ret = "Moved Permanently"; break;
		case 302: ret = "Found"; break;
		case 303: ret = "See Other"; break;
		case 304: ret = "Not Modified"; break;
		case 305: ret = "Use Proxy"; break;
		case 307: ret = "Temporary Redirect"; break;
		case 400: ret = "Bad Request"; break;
		case 401: ret = "Unauthorized"; break;
		case 402: ret = "Payment Required"; break;
		case 403: ret = "Forbidden"; break;
		case 404: ret = "Not Found"; break;
		case 405: ret = "Method Not Allowed"; break;
		case 406: ret = "Not Acceptable"; break;
		case 407: ret = "Proxy Authentication Required"; break;
		case 408: ret = "Request Time out"; break;
		case 409: ret = "Conflict"; break;
		case 410: ret = "Gone"; break;
		case 411: ret = "Length Required"; break;
		case 412: ret = "Precondition Failed"; break;
		case 413: ret = "Request Entity Too Large"; break;
		case 414: ret = "Request URI Too Large"; break;
		case 415: ret = "Unsupported Media Type"; break;
		case 416: ret = "Requested range not satisfiable"; break;
		case 417: ret = "Expectation Failed"; break;
		case 500: ret = "Internal Server Error"; break;
		case 501: ret = "Not Implemented"; break;
		case 502: ret = "Bad Gateway"; break;
		case 503: ret = "Server Unavailable"; break;
		case 504: ret = "Gateway Time out"; break;
		case 505: ret = "HTTP Version not supported"; break;
		default: ret = "Unknow Code"; break;
		}

		return ret;
	}
}
