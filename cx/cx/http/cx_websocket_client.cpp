#include "cx_websocket_client.h"
#include "../common/cx_guid.h"
#include "cx_http_response.h"

namespace cx
{
	cxWebSocketClient::cxWebSocketClient()
	{
		
	}
	cxWebSocketClient::~cxWebSocketClient()
	{

	}
	cxWebSocketClient::Ptr cxWebSocketClient::CreatePtr()
	{
		cxWebSocketClient::Ptr ptr = std::make_shared<cxWebSocketClient>();
		return ptr;
	}
	bool cxWebSocketClient::IsConnected()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL || !m_socket->IsConnected())
		{
			return false;
		}
		return true;
	}
	void cxWebSocketClient::Close()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL)
		{
			m_socket->Close();
		}
	}
	int cxWebSocketClient::ConnectServer(std::string ws_url, unsigned int time_out)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);

		if (m_socket != NULL && m_socket->IsConnected() || ws_url == "")
		{
			return -1;
		}

		m_socket = cxSocketTcp::CreateSocket();
		if (m_socket == NULL)
		{
			return -1;
		}

		std::string url = "";
		std::string host = "";
		int port = 0;

		if (!_ParseRequestUrl(ws_url, url, host, port))
		{
			m_socket->Close();
			return -1;
		}

		if (!m_socket->Connect(host, port, time_out))
		{
			m_socket->Close();
			return -1;
		}

		cxGuid guid;
		std::string token = guid.toToken().substr(0, 16);
		m_Sec_WebSocket_Key = cxCommonFun::Base64_Encodec((char*)token.data(), token.size());

		std::stringstream ss;
		ss << "GET " << url << " HTTP/1.1" << "\r\n";
		ss << "Host: " << host << ":" << port << "\r\n";
		ss << "Connection: Upgrade\r\n";
		ss << "Upgrade: websocket\r\n";
		ss << "Sec-WebSocket-Version: 13\r\n";
		ss << "Sec-WebSocket-Key: " << m_Sec_WebSocket_Key << "\r\n";
		ss << "\r\n";

		std::string str_req = ss.str();

		if (!m_socket->Send((void*)str_req.data(), str_req.size()))
		{
			m_socket->Close();
			return -1;
		}

		char buf[16384];
		int len = 0;
		if (!m_socket->Recv((void*)buf, sizeof(buf), len) || len <= 0)
		{
			m_socket->Close();
			return -1;
		}

		std::string str_res(buf, buf + len);
		if (str_res == "")
		{
			m_socket->Close();
			return -1;
		}

		int index = str_res.find("\r\n\r\n");
		if (index <= 0)
		{
			m_socket->Close();
			return -1;
		}

		cxHttpStream::Ptr ptr;
		cxHttpResponse::Ptr http_res = std::make_shared<cxHttpResponse>(ptr);
		if (!http_res->ParseHeaderString(str_res.substr(0, index)))
		{
			m_socket->Close();
			return -1;
		}

		if (http_res->GetCode() != 101)
		{
			m_socket->Close();
			return http_res->GetCode();
		}

		std::string res_key = m_Sec_WebSocket_Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::vector<char> sha_req(res_key.begin(), res_key.end());
		std::vector<char> sha = cxCommonFun::Sha1(sha_req);
		res_key = cxCommonFun::Base64_Encodec(sha);

		if (http_res->GetHeader<std::string>("Sec-WebSocket-Accept") != res_key)
		{
			m_socket->Close();
			return -1;
		}

		m_recv_buf.clear();

		if (len > str_res.size())
		{
			m_recv_buf.insert(m_recv_buf.end(), buf + str_res.size(), buf + len);
		}

		return http_res->GetCode();
	}
	bool cxWebSocketClient::SendData(std::string data)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x81);
		if (data.size() < 126)
		{
			buf.push_back(data.size());
		}
		else if (data.size() <= 65535)
		{
			buf.push_back(0x7E);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)data.size());
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)data.size());
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}
		if (data.size() > 0)
		{
			buf.insert(buf.end(), data.begin(), data.end());
		}
		return m_socket->Send((void*)buf.data(), buf.size());
	}
	bool cxWebSocketClient::SendData(void* data, unsigned int data_size)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x82);
		if (data_size < 126)
		{
			buf.push_back(data_size);
		}
		else if (data_size <= 65535)
		{
			buf.push_back(0x7E);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)data_size);
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)data_size);
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}
		if (data_size > 0)
		{
			buf.insert(buf.end(), (unsigned char*)data, (unsigned char*)data + data_size);
		}
		return m_socket->Send((void*)buf.data(), buf.size());
	}
	bool cxWebSocketClient::SendPing()
	{
		unsigned char buf[2] = { 0x89, 0x00 };
		return m_socket->Send((void*)buf, sizeof(buf));
	}
	bool cxWebSocketClient::SendPong()
	{
		unsigned char buf[2] = { 0x8A, 0x00 };
		return m_socket->Send((void*)buf, sizeof(buf));
	}
	bool cxWebSocketClient::Recv(WSDATATYPE& type, std::shared_ptr<std::vector<char>>& data, unsigned int time_out, bool auto_ping)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL || !m_socket->IsConnected())
		{
			return false;
		}

		while (true)
		{
			if (m_recv_buf.size() < 2)
			{
				if (!_RecvData(time_out))
				{
					m_socket->Close();
					return false;
				}
				continue;
			}

			bool fin = 0;
			int reserved = 0;
			int opcode = 0;
			bool mask = false;
			unsigned char mask_key[4];
			unsigned long long payload_length = 0;
			int index = 0;

			unsigned int buf_size = m_recv_buf.size();

			// 解析第一个字节
			int dd = m_recv_buf.data()[index];
			fin = (m_recv_buf.data()[index] >> 7) & 0x01;
			reserved = (m_recv_buf.data()[index] >> 4) & 0x07;
			opcode = m_recv_buf.data()[index] & 0x0F;
			index++;

			// 解析第二个字节
			mask = (m_recv_buf.data()[index] >> 7) & 0x01;
			payload_length = m_recv_buf.data()[index] & 0x7F;
			if (payload_length == 126)
			{
				if (index + 2 >= buf_size)
				{
					if (!_RecvData(time_out))
					{
						m_socket->Close();
						return false;
					}
					continue;
				}
				payload_length = Net2Host16((unsigned char*)m_recv_buf.data() + index + 1);
				index += 2;
			}
			else if (payload_length == 127)
			{
				if (index + 8 >= buf_size)
				{
					if (!_RecvData(time_out))
					{
						m_socket->Close();
						return false;
					}
					continue;
				}
				payload_length = Net2Host64((unsigned char*)m_recv_buf.data() + index + 1);
				index += 8;
			}
			index++;

			// 获取掩码
			if (mask)
			{
				if (buf_size < index + 4)
				{
					if (!_RecvData(time_out))
					{
						m_socket->Close();
						return false;
					}
					continue;
				}
				memcpy(mask_key, m_recv_buf.data() + index, 4);
				index += 4;
			}

			if (buf_size < index + payload_length)
			{
				if (!_RecvData(time_out))
				{
					m_socket->Close();
					return false;
				}
				continue;
			}

			// 使用掩码还原数据
			if (mask && payload_length > 0)
			{
				unsigned char* ppay = (unsigned char*)m_recv_buf.data() + index;
				for (unsigned long long i = 0, j = 0; i < payload_length; i++, j++)
				{
					if (j >= 4)
					{
						j = 0;
					}

					ppay[i] ^= mask_key[j];
				}
			}

			// 分片起始消息
			if (!fin && opcode != 0)
			{
				m_ws_temp.clear();
				m_temp_opcode = opcode;
				if (payload_length > 0)
				{
					m_ws_temp.insert(m_ws_temp.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
				}
				m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
				continue;
			}
			// 分片消息
			else if (!fin && opcode == 0)
			{
				if (payload_length > 0)
				{
					m_ws_temp.insert(m_ws_temp.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
				}
				m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
				continue;
			}
			// 分片结束消息
			else if (fin && opcode == 0)
			{
				if (payload_length > 0)
				{
					m_ws_temp.insert(m_ws_temp.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
					data = std::make_shared<std::vector<char>>(m_ws_temp.begin(), m_ws_temp.end());
				}
				m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
				opcode = m_temp_opcode;
			}
			// 正常消息
			else if (fin && opcode != 0)
			{
				if (payload_length > 0)
				{
					data = std::make_shared<std::vector<char>>(m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
				}
				m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
			}

			switch (opcode)
			{
			case 0x00: // continuation frame
				break;
			case 0x01: // text frame
				type = WSDATATYPE::string;
				break;
			case 0x02: // binary frame
				type = WSDATATYPE::binary;
				break;
			case 0x08: // connection close
				m_socket->Close();
				return false;
			case 0x09: // ping frame
				type = WSDATATYPE::ping;
				break;
			case 0x0A: // pong frame
				type = WSDATATYPE::pong;
				break;
			}

			if (auto_ping && type == WSDATATYPE::ping)
			{
				unsigned char buf[2] = { 0x8A, 0x00 };
				if (!m_socket->Send((void*)buf, sizeof(buf)))
				{
					m_socket->Close();
					return false;
				}
				data = NULL;
				continue;
			}

			return true;
		}
	}

	bool cxWebSocketClient::_RecvData(unsigned int time_out)
	{
		char buf[16384];
		int len = 0;
		if (!m_socket->Recv((void*)buf, sizeof(buf), len))
		{
			return false;
		}		
		m_recv_buf.insert(m_recv_buf.end(), buf, buf + len);
		return true;
	}
	bool cxWebSocketClient::_ParseRequestUrl(std::string ws_url, std::string& url, std::string& host, int& port)
	{
		int index = ws_url.find("ws://");
		if (index != 0)
		{
			return false;
		}
		ws_url = ws_url.substr(5);
		index = ws_url.find("/");
		if (index > 0)
		{
			url = ws_url.substr(index);
			ws_url = ws_url.substr(0, index);
		}
		else
		{
			url = "/";
		}

		std::vector<std::string> group;
		cxCommonFun::StringToGroup(ws_url, ":", group);
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

		return true;
	}
}
