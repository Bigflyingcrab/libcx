#include "cx_websocket.h"
#include "../common/cx_guid.h"
#include "cx_http_response.h"
#include "cx_http_request.h"

namespace cx
{
	cxWebSocket::cxWebSocket(cxSocketTcp::Ptr sock)
	{
		Init();
		m_socket = sock;
	}
	cxWebSocket::~cxWebSocket()
	{
		if (m_socket != NULL)
		{
			m_socket->Close();
		}
	}
	std::string cxWebSocket::GetIp()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL)
		{
			return "";
		}
		return m_socket->GetIp();
	}
	int cxWebSocket::GetPort()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL)
		{
			return 0;
		}
		return m_socket->GetPort();
	}
	SocketFD cxWebSocket::GetFD()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL)
		{
			return -1;
		}
		return m_socket->GetFD();
	}
	bool cxWebSocket::IsConnected()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket == NULL)
		{
			return false;
		}
		return m_socket->IsConnected();
	}
	void cxWebSocket::Close()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		_Close();
	}
	int cxWebSocket::Connect(std::string url, On_Message_Str cb_str, On_Message_Bin cb_bin, On_Close cb_close, long long time_out)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL && m_socket->IsConnected())
		{
			return -1;
		}
		
		std::string ws_url = "";
		std::string host = "";
		int port = 0;
		if (!ParseRequestUrl(url, ws_url, host, port))
		{
			return -1;
		}

		m_socket = cxSocketTcp::CreateSocket();
		if (m_socket == NULL)
		{
			return -1;
		}

		if (!m_socket->Connect(host, port, time_out))
		{
			return -1;
		}

		cxGuid guid;
		std::string token = guid.toToken().substr(0, 16);
		m_Sec_WebSocket_Key = cxCommonFun::Base64_Encodec((char*)token.data(), token.size());

		std::stringstream ss;
		ss << "GET " << ws_url << " HTTP/1.1" << "\r\n";
		ss << "Host: " << host << ":" << port << "\r\n";
		ss << "Connection: Upgrade\r\n";
		ss << "Upgrade: websocket\r\n";
		ss << "Sec-WebSocket-Version: 13\r\n";
		ss << "Sec-WebSocket-Key: " << m_Sec_WebSocket_Key << "\r\n";
		ss << "\r\n";

		m_url = ws_url;
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

		if (http_res->GetCode() == 404)
		{
			m_socket->Close();
			return 1;
		}
		else if (http_res->GetCode() == 401)
		{
			m_socket->Close();
			return 2;
		}
		else if (http_res->GetCode() != 101)
		{
			m_socket->Close();
			return -1;
		}

		m_connected = true;
		g_clients.set(m_socket->GetFD(), shared_from_this());
		m_cb_message_str = cb_str;
		m_cb_message_bin = cb_bin;
		m_cb_close = cb_close;
		m_socket->BeginRecv(OnRecv);

		return 0;
	}
	bool cxWebSocket::StartListen(std::string ip, int port, std::string url, On_Accept cb)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);

		if (m_socket != NULL && m_socket->IsConnected())
		{
			return false;
		}

		m_socket = cxSocketTcp::CreateSocket();
		if (m_socket == NULL)
		{
			return false;
		}

		m_url = url;
		g_servers.set(m_socket->GetFD(), shared_from_this());
		if (!m_socket->BeginListen(ip, port, OnAccept))
		{
			m_socket->Close();
			g_servers.set(m_socket->GetFD(), NULL);
			return false;
		}
		m_cb_accept = cb;
		return true;
	}
	bool cxWebSocket::Send(void* data, int len)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		return _Send(data, len, false, m_server == NULL);
	}
	bool cxWebSocket::Send(std::string str)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		return _Send((void*)str.data(), str.size(), true, m_server == NULL);
	}
	bool cxWebSocket::Send(Json::Value json)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (json.isNull())
		{
			return true;
		}
		std::string str = json.toStyledString();
		//cxLogerEnable(str);
		return _Send((void*)str.data(), str.size(), true, m_server == NULL);
	}
	void cxWebSocket::SetOnMessageStr(On_Message_Str cb)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		m_cb_message_str = cb;
	}
	void cxWebSocket::SetOnMessageBin(On_Message_Bin cb)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		m_cb_message_bin = cb;
	}
	void cxWebSocket::SetOnClose(On_Close cb)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		m_cb_close = cb;
	}
	bool cxWebSocket::_Ping()
	{
		if (m_socket == NULL)
		{
			return false;
		}

		bool bclient = m_server == NULL;
		unsigned char mask = (bclient ? 0x80 : 0x00);
		std::vector<unsigned char> buf;
		buf.push_back(0x89);
		int len = 0;

		if (len < 126)
		{
			buf.push_back((unsigned char)len | mask);
		}
		else if (len <= 65535)
		{
			buf.push_back(0x7E | mask);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)len);
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F | mask);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)len);
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}

		if (bclient)
		{
			std::vector<unsigned char> mask_list;
			for (int i = 0; i < 4; i++)
			{
				unsigned char temp = (unsigned char)cx::cxCommonFun::CreateRandNum(0, 255);
				buf.push_back(temp);
				mask_list.push_back(temp);
			}
		}

		return m_socket->Send((void*)buf.data(), buf.size());
	}
	void cxWebSocket::_Close()
	{
		if (m_socket != NULL)
		{
			m_socket->Close();
		}
	}
	bool cxWebSocket::_Send(void* data, int len, bool bstr, bool bclient)
	{
		if (m_socket == NULL)
		{
			return false;
		}

		unsigned char mask = (bclient ? 0x80 : 0x00);

		std::vector<unsigned char> buf;
		if (bstr)
		{
			buf.push_back(0x81);
		}
		else
		{
			buf.push_back(0x82);
		}		
		if (len < 126)
		{
			buf.push_back((unsigned char)len | mask);
		}
		else if (len <= 65535)
		{
			buf.push_back(0x7E | mask);
			unsigned char temp[2];
			Host2Net(temp, (unsigned short)len);
			buf.push_back(temp[0]);
			buf.push_back(temp[1]);
		}
		else
		{
			buf.push_back(0x7F | mask);
			unsigned char temp[8];
			Host2Net(temp, (unsigned long long)len);
			for (unsigned int i = 0; i < 8; i++)
			{
				buf.push_back(temp[i]);
			}
		}

		if (bclient)
		{
			std::vector<unsigned char> mask_list;
			for (int i = 0; i < 4; i++)
			{
				unsigned char temp = (unsigned char)cx::cxCommonFun::CreateRandNum(0, 255);
				buf.push_back(temp);
				mask_list.push_back(temp);
			}

			if (len > 0)
			{
				int index = 0;
				for (int i = 0; i < len; i++, index++)
				{
					if (index == 4)
					{
						index = 0;
					}
					buf.push_back(((unsigned char*)data)[i] ^ mask_list[index]);
				}
			}
		}
		else
		{
			for (int i = 0; i < len; i++)
			{
				buf.push_back(((unsigned char*)data)[i]);
			}
		}

		return m_socket->Send((void*)buf.data(), buf.size());
	}
	int cxWebSocket::_ParseWSData()
	{
		if (m_recv_buf.size() <= 0)
		{
			return 1;
		}

		int ret = 0;
		if (!m_connected)
		{
			ret = _ParseWSData_Http();
			if (ret < 0)
			{
				return ret;
			}
		}
		
		if (m_connected)
		{
			ret = _ParseWSData_WS();
		}

		return ret;
	}
	int cxWebSocket::_ParseWSData_Http()
	{
		if (m_connected)
		{
			return 0;
		}
		std::string str_req((char*)m_recv_buf.data(), (char*)m_recv_buf.data() + m_recv_buf.size());
		int index = str_req.find("\r\n\r\n");
		if (index < 0)
		{
			if (m_recv_buf.size() >= 8192)
			{
				Close();
				return -1;
			}
			else
			{
				return 1;
			}
		}

		str_req = str_req.substr(0, index + 4);
		m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + str_req.size());

		cx::cxHttpRequest::Ptr req = std::make_shared<cx::cxHttpRequest>();
		if (!req->ParseHeaderString(str_req))
		{
			Close();
			return -1;
		}

		if (req->GetUrl() != m_url)
		{
			Close();
			return -1;
		}

		if (req->GetMode() == cx::cxHttp_Mode::OPTIONS)
		{
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK\r\n";
			ss << "Access-Control-Allow-Origin: *\r\n";
			ss << "Access-Control-Allow-Headers: *\r\n";
			ss << "Access-Control-Allow-Methods: *\r\n";
			ss << "Server: libcx\r\n";
			ss << "\r\n";
			std::string str_res = ss.str();
			m_socket->Send((void*)str_res.data(), str_res.size());
			Close();
			return -1;
		}
		else if (req->GetMode() != cx::cxHttp_Mode::GET ||
			req->GetHeader<std::string>("Upgrade") != "websocket" ||
			req->GetHeader<std::string>("Connection") != "Upgrade")
		{
			Close();
			return -1;
		}

		std::string key = req->GetHeader<std::string>("Sec-WebSocket-Key");
		if (key == "")
		{
			Close();
			return -1;
		}

		std::string res_key = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::vector<char> sha_req(res_key.begin(), res_key.end());
		std::vector<char> sha = cx::cxCommonFun::Sha1(sha_req);
		res_key = cx::cxCommonFun::Base64_Encodec(sha);

		std::stringstream ss;
		ss << "HTTP/1.1 101 Switching Protocols\r\n";
		ss << "Access-Control-Allow-Origin: *\r\n";
		ss << "Access-Control-Allow-Headers: *\r\n";
		ss << "Access-Control-Allow-Methods: *\r\n";
		ss << "Server: libcx\r\n";
		ss << "Sec-WebSocket-Accept: " << res_key << "\r\n";
		ss << "Upgrade: websocket\r\n";
		ss << "Connection: Upgrade\r\n";
		ss << "\r\n";
		std::string str_res = ss.str();
		if (!m_socket->Send((void*)str_res.data(), str_res.size()))
		{
			Close();
			return -1;
		}

		m_connected = true;

		if (m_cb_accept != NULL)
		{
			m_cb_accept(m_server, shared_from_this());
		}
		return 0;
	}
	int cxWebSocket::_ParseWSData_WS()
	{
		if (m_recv_buf.size() <= 1)
		{
			return 1;
		}

		bool fin = 0;
		int reserved = 0;
		int opcode = 0;
		bool mask = false;
		unsigned char mask_key[4];
		unsigned long long payload_length = 0;
		int index = 0;

		// 解析第一个字节
		fin = (m_recv_buf.data()[index] >> 7) & 0x01;
		reserved = (m_recv_buf.data()[index] >> 4) & 0x07;
		opcode = m_recv_buf.data()[index] & 0x0F;
		index++;

		// 解析第二个字节
		mask = (m_recv_buf.data()[index] >> 7) & 0x01;
		payload_length = m_recv_buf.data()[index] & 0x7F;
		if (payload_length == 126)
		{
			if (index + 2 >= m_recv_buf.size())
			{
				return 1;
			}
			payload_length = cx::Net2Host16((unsigned char*)m_recv_buf.data() + index + 1);
			index += 2;
		}
		else if (payload_length == 127)
		{
			if (index + 8 >= m_recv_buf.size())
			{
				return 1;
			}
			payload_length = cx::Net2Host64((unsigned char*)m_recv_buf.data() + index + 1);
			index += 8;
		}
		index++;

		// 获取掩码
		if (mask)
		{
			if (m_recv_buf.size() < index + 4)
			{
				return 1;
			}
			memcpy(mask_key, m_recv_buf.data() + index, 4);
			index += 4;
		}

		if (m_recv_buf.size() < index + payload_length)
		{
			return 1;
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
			m_opcode_buf.clear();
			m_opcode_type = opcode;
			if (payload_length > 0)
			{
				m_opcode_buf.insert(m_opcode_buf.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
			}
			m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
			return 1;
		}
		// 分片消息
		else if (!fin && opcode == 0)
		{
			if (payload_length > 0)
			{
				m_opcode_buf.insert(m_opcode_buf.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
			}
			m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
			return 1;
		}
		// 分片结束消息
		else if (fin && opcode == 0)
		{
			if (payload_length > 0)
			{
				m_opcode_buf.insert(m_opcode_buf.end(), m_recv_buf.begin() + index, m_recv_buf.begin() + index + payload_length);
			}
			_WSMessage(m_opcode_type, m_opcode_buf.data(), m_opcode_buf.size());
			m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
		}
		// 正常消息
		else if (fin && opcode != 0)
		{
			_WSMessage(opcode, m_recv_buf.data() + index, payload_length);
			m_recv_buf.erase(m_recv_buf.begin(), m_recv_buf.begin() + index + payload_length);
		}

		return _ParseWSData_WS();
	}
	void cxWebSocket::_WSMessage(int op, void* data, int len)
	{
		switch (op)
		{
		case 0x00: // continuation frame
			break;
		case 0x01: // text frame
			_WSMessage_String(data, len);
			break;
		case 0x02: // binary frame
			_WSMessage_Bin(data, len);
			break;
		case 0x08: // connection close
			Close();
			return;
		case 0x09: // ping frame
			_WSMessage_Ping();
			break;
		case 0x0A: // pong frame
			_WSMessage_Pong();
			break;
		}
	}
	void cxWebSocket::_WSMessage_String(void* data, int len)
	{
		std::string str((char*)data, len);
		On_Message_Str cb = m_cb_message_str;
		if (cb != NULL)
		{
			//cxLogerEnable(str);
			cb(shared_from_this(), str);
		}
	}
	void cxWebSocket::_WSMessage_Bin(void* data, int len)
	{
		On_Message_Bin cb = m_cb_message_bin;
		if (cb != NULL)
		{
			cb(shared_from_this(), data, len);
		}
	}
	void cxWebSocket::_WSMessage_Ping()
	{
		unsigned char buf[2] = { 0x8A, 0x00 };
		m_socket->Send((void*)buf, 2);
	}
	void cxWebSocket::_WSMessage_Pong()
	{

	}

	cxSafeMap<SocketFD, std::shared_ptr<cxWebSocket>>		cxWebSocket::g_servers;
	cxSafeMap<SocketFD, std::shared_ptr<cxWebSocket>>		cxWebSocket::g_clients;

	void cxWebSocket::OnAccept(cxSocketTcp::Ptr server, cxSocketTcp::Ptr client)
	{
		std::shared_ptr<cxWebSocket> ptr_server = g_servers.get(server->GetFD());
		if (ptr_server == NULL)
		{
			return;
		}

		std::shared_ptr<cxWebSocket> ptr = std::make_shared<cxWebSocket>(client);
		ptr->m_cb_accept = ptr_server->m_cb_accept;
		ptr->m_server = ptr_server;
		ptr->m_url = ptr_server->m_url;
		g_clients.set(client->GetFD(), ptr);
		client->SetOnClose(OnClose);
		client->BeginRecv(OnRecv);
	}
	void cxWebSocket::OnRecv(cxSocketTcp::Ptr client, void* data, unsigned int size)
	{
		std::shared_ptr<cxWebSocket> ptr = g_clients.get(client->GetFD());
		if (ptr == NULL)
		{
			return;
		}

		if (data != NULL && size > 0)
		{
			ptr->m_recv_buf.insert(ptr->m_recv_buf.end(), (unsigned char*)data, (unsigned char*)data + size);
		}

		if (ptr->_ParseWSData() >= 0)
		{
			client->BeginRecv(OnRecv);
		}
	}
	void cxWebSocket::OnClose(cxSocketTcp::Ptr client)
	{
		std::shared_ptr<cxWebSocket> ptr = g_clients.get(client->GetFD());
		if (ptr != NULL)
		{
			g_clients.set(client->GetFD(), ptr);
			On_Close cb = ptr->m_cb_close;
			if (cb != NULL)
			{
				cb(ptr);
			}
		}
	}
	bool cxWebSocket::ParseRequestUrl(std::string ws_url, std::string& url, std::string& host, int& port)
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

	bool														cxWebSocket::g_init = false;
	std::mutex													cxWebSocket::g_init_mutex;
	void cxWebSocket::Init()
	{
		if (g_init)
		{
			return;
		}
		std::unique_lock<std::mutex> Lock(g_init_mutex);
		if (g_init)
		{
			return;
		}

		std::thread td(Thread_Heart);
		td.detach();
		g_init = true;
	}
	void cxWebSocket::Thread_Heart()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			for (int i = 0; i < g_clients.size(); i++)
			{
				std::shared_ptr<cxWebSocket> client = g_clients.get_value(i);
				if (client != NULL)
				{
					client->_Ping();
				}
			}
		}
	}
}
