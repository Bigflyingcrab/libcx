#include "cx_http_server.h"
#include "../thread/cx_thread_threadpool.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxHttpServer::cxHttpServer()
	{

	}
	cxHttpServer::~cxHttpServer()
	{

	}
	bool cxHttpServer::SetWebPath(const std::string& path)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL && m_socket->IsConnected())
		{
			cxLogerError("设置http服务器web路径失败，服务已经启动");
			return false;
		}
		m_web_path = path;
		if (m_web_path != "")
		{
			m_web_controller = std::make_shared<cxHttpController>(m_web_path);
		}
		else
		{
			m_web_controller = NULL;
		}
		return true;
	}
	bool cxHttpServer::AddController(const std::string& url, std::shared_ptr<cxWebController> controller)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (url == "" || controller == NULL || m_socket != NULL && m_socket->IsConnected())
		{
			cxLogerError("添加http服务器控制器失败，参数有误或者服务已经启动");
			return false;
		}

		m_user_controller[url] = controller;
		return true;
	}
	bool cxHttpServer::StartServer(const std::string& ip, int port)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL && m_socket->IsConnected())
		{
			cxLogerError("启动http服务失败，服务已经启动");
			return false;
		}

		m_socket = cxSocketTcp::CreateSocket();
		if (m_socket == NULL)
		{
			cxLogerError("启动http服务失败，创建socket失败");
			return false;
		}

		if (!m_socket->BeginListen(ip, port, OnAccept))
		{
			cxLogerError("启动http服务失败，启动监听失败");
			return false;
		}

		g_servers_mutex.lock();
		g_servers[m_socket->GetFD()] = shared_from_this();
		g_servers_mutex.unlock();
		m_socket->SetOnClose(OnClose);

		cxLogerEnable("http服务[" << ip << ":" << port << "][" << m_web_path << "]启动成功");

		return true;
	}
	bool cxHttpServer::IsRunning()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL && m_socket->IsConnected())
		{
			return true;
		}
		return false;
	}
	void cxHttpServer::CloseServer()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_socket != NULL)
		{
			m_socket->Close();
		}
	}

	std::map<SocketFD, cxHttpServer::Ptr>				cxHttpServer::g_servers;
	std::mutex											cxHttpServer::g_servers_mutex;
	cxSafeMap<SocketFD, std::shared_ptr<cxHttpStream>>	cxHttpServer::g_streams;

	void cxHttpServer::OnAccept(cxSocketTcp::Ptr server, cxSocketTcp::Ptr sock)
	{
		cxHttpStream::Ptr stream = std::make_shared<cxHttpStream>(sock, server->GetFD());
		g_streams.set(sock->GetFD(), stream);
		sock->SetTimeout(20000);
		sock->SetOnClose(OnClose);
		if (!stream->m_socket->BeginRecv(OnRecv))
		{
			g_streams.set(sock->GetFD(), NULL);
		}
	}
	void cxHttpServer::OnRecv(cxSocketTcp::Ptr sock, void* data, unsigned int size)
	{
		cxHttpStream::Ptr stream = g_streams.get(sock->GetFD());
		if (stream == NULL)
		{
			sock->Close();
			return;
		}

		stream->m_recv_buf.push_back(std::make_shared<std::vector<unsigned char>>((unsigned char*)data, (unsigned char*)data + size));

		if (stream->m_header_string == "")
		{
			int ret = stream->ParseHeader();
			if (ret < 0)
			{
				cxLogerError("解析http数据失败，获取http header数据失败");
				stream->Close();
				return;
			}
			else if (ret > 0)
			{
				stream->m_socket->BeginRecv(OnRecv);
				return;
			}

			cxHttpServer::Ptr server = g_servers[stream->m_server_fd];
			if (server == NULL)
			{
				cxLogerError("解析http数据失败，未找到对应的http server");
				stream->Close();
				return;
			}

			cxHttpRequest::Ptr request = std::make_shared<cxHttpRequest>(stream);
			if (!request->ParseHeaderString(stream->m_header_string))
			{
				cxLogerError("解析http数据失败，未能成功解析header数据");
				stream->Close();
				return;
			}

			std::shared_ptr<cxWebController> controller = server->m_user_controller[request->GetUrl()];
			if (controller == NULL)
			{
				controller = server->m_web_controller;
			}
			if (controller == NULL)
			{
				cxHttpResponse::Ptr response = std::make_shared<cxHttpResponse>(stream);
				response->Response(404);
				return;
			}

			if (controller->IsWebSocket())
			{
				cxThreadPool::RunThread(DoWebSocketController, controller, request);
				//DoWebSocketController(controller, request);
			}
			else
			{
				cxThreadPool::RunThread(DoHttpController, controller, request);
				//DoHttpController(controller, request);
			}
		}
		else
		{
			if (stream->m_bws)
			{
				ParseWebSocketData(stream);
				stream->m_socket->BeginRecv(OnRecv);
			}
		}
	}
	void cxHttpServer::DoHttpController(std::shared_ptr<cxWebController> controller, cxHttpRequest::Ptr request)
	{
		cxHttpResponse::Ptr response = std::make_shared<cxHttpResponse>(request->Stream());
		if (response == NULL)
		{
			cxLogerError("执行http控制器失败，未能成功创建response实体");
			request->Stream()->Close();
			return;
		}
		if (controller->GetCross())
		{
			response->SetHeader("Access-Control-Allow-Origin", "*");
			response->SetHeader("Access-Control-Allow-Headers", "*");
			response->SetHeader("Access-Control-Allow-Methods", "*");
		}

		switch (request->GetMode())
		{
			case cxHttp_Mode::GET: controller->GET(request, response); break;
			case cxHttp_Mode::HEAD: controller->HEAD(request, response); break;
			case cxHttp_Mode::POST: controller->POST(request, response); break;
			case cxHttp_Mode::OPTIONS: controller->OPTIONS(request, response); break;
			case cxHttp_Mode::PUT: controller->PUT(request, response); break;
			case cxHttp_Mode::DELETE: controller->DELETE(request, response); break;
			case cxHttp_Mode::TRACE: controller->TRACE(request, response); break;
			case cxHttp_Mode::CONNECT: controller->CONNECT(request, response); break;
			default: response->Response(405); break;
		}

		request->Stream()->Close();
		return;

		if (request->GetHeader<std::string>("Connection") == "keep-alive")
		{
			request->Stream()->Complete();
			request->Stream()->m_socket->BeginRecv(OnRecv);
		}
		else
		{
			request->Stream()->Close();
		}
	}
	void cxHttpServer::DoWebSocketController(std::shared_ptr<cxWebController> controller, cxHttpRequest::Ptr request)
	{
		cxHttpResponse::Ptr response = std::make_shared<cxHttpResponse>(request->Stream());
		if (response == NULL)
		{
			cxLogerError("执行websocket控制器失败，未能成功创建response实体");
			request->Stream()->Close();
			return;
		}

		if (request->GetMode() != cxHttp_Mode::GET ||
			request->GetHeader<std::string>("Upgrade") != "websocket" ||
			request->GetHeader<std::string>("Connection") != "Upgrade")
		{
			cxLogerError("执行websocket控制器失败，websocket的http请求不合法");
			response->Response(400);
			request->Stream()->Close();
			return;
		}
		std::string key = request->GetHeader<std::string>("Sec-WebSocket-Key");
		if (key == "")
		{
			cxLogerError("执行websocket控制器失败，Sec-WebSocket-Key是空");
			response->Response(400);
			request->Stream()->Close();
			return;
		}

		std::string res_key = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		std::vector<char> sha_req(res_key.begin(), res_key.end());
		std::vector<char> sha = cxCommonFun::Sha1(sha_req);
		res_key = cxCommonFun::Base64_Encodec(sha);

		response->SetHeader("Sec-WebSocket-Accept", res_key);
		response->SetHeader("Upgrade", "websocket");
		response->SetHeader("Connection", "Upgrade");
		
		if (!response->Response(101))
		{
			cxLogerError("执行websocket控制器失败，发送应答数据失败");
			request->Stream()->Close();
			return;
		}

		request->Stream()->m_url = request->GetUrl();
		request->Stream()->m_bws = true;

		//cxLogerEnable("websocket客户端[" << request->Stream()->m_socket->GetIp() << ":" << request->Stream()->m_socket->GetPort() << "]接入");

		controller->WS_OnAccept(request->Stream());

		ParseWebSocketData(request->Stream());

		request->Stream()->m_socket->BeginRecv(OnRecv);
	}
	void cxHttpServer::ParseWebSocketData(cxHttpStream::Ptr stream)
	{
		if (stream->m_recv_buf.empty())
		{
			return;
		}

		while (stream->m_recv_buf.size() > 1)
		{
			std::shared_ptr<std::vector<unsigned char>> temp = stream->m_recv_buf.front();
			stream->m_recv_buf.pop_front();
			if (temp != NULL && temp->size() > 0)
			{
				stream->m_recv_buf.front()->insert(stream->m_recv_buf.front()->begin(), temp->begin(), temp->end());
			}
		}

		std::shared_ptr<std::vector<unsigned char>> buf = stream->m_recv_buf.front();
		unsigned int buf_size = buf->size();
		if (buf_size < 2)
		{
			return;
		}

		bool fin = 0;
		int reserved = 0;
		int opcode = 0;
		bool mask = false;
		unsigned char mask_key[4];
		unsigned long long payload_length = 0;
		int index = 0;

		// 解析第一个字节
		fin = (buf->data()[index] >> 7) & 0x01;
		reserved = (buf->data()[index] >> 4) & 0x07;
		opcode = buf->data()[index] & 0x0F;
		index++;

		// 解析第二个字节
		mask = (buf->data()[index] >> 7) & 0x01;
		payload_length = buf->data()[index] & 0x7F;
		if (payload_length == 126)
		{
			if (index + 2 >= buf_size)
			{
				return;
			}
			payload_length = Net2Host16((unsigned char*)buf->data() + index + 1);
			index += 2;
		}
		else if (payload_length == 127)
		{
			if (index + 8 >= buf_size)
			{
				return;
			}
			payload_length = Net2Host64((unsigned char*)buf->data() + index + 1);
			index += 8;
		}
		index++;

		// 获取掩码
		if (mask)
		{
			if (buf_size < index + 4)
			{
				return;
			}
			memcpy(mask_key, buf->data() + index, 4);
			index += 4;
		}

		if (buf_size < index + payload_length)
		{
			return;
		}

		// 使用掩码还原数据
		if (mask && payload_length > 0)
		{
			unsigned char* ppay = (unsigned char*)buf->data() + index;
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
			stream->m_ws_temp.clear();
			stream->m_temp_opcode = opcode;
			if (payload_length > 0)
			{
				stream->m_ws_temp.insert(stream->m_ws_temp.end(), buf->begin() + index, buf->begin() + index + payload_length);
			}
			buf->erase(buf->begin(), buf->begin() + index + payload_length);			
			return;
		}
		// 分片消息
		else if (!fin && opcode == 0)
		{
			if (payload_length > 0)
			{
				stream->m_ws_temp.insert(stream->m_ws_temp.end(), buf->begin() + index, buf->begin() + index + payload_length);
			}
			buf->erase(buf->begin(), buf->begin() + index + payload_length);
			return;
		}
		// 分片结束消息
		else if (fin && opcode == 0)
		{
			if (payload_length > 0)
			{
				stream->m_ws_temp.insert(stream->m_ws_temp.end(), buf->begin() + index, buf->begin() + index + payload_length);
			}
			DoWebSocketController_Message(stream, opcode, stream->m_ws_temp.data(), stream->m_ws_temp.size());
			buf->erase(buf->begin(), buf->begin() + index + payload_length);
		}
		// 正常消息
		else if (fin && opcode != 0)
		{
			DoWebSocketController_Message(stream, opcode, (void*)(buf->data() + index), payload_length);
			buf->erase(buf->begin(), buf->begin() + index + payload_length);
		}

		if (buf->size() >= 2)
		{
			return ParseWebSocketData(stream);
		}
	}
	void cxHttpServer::DoWebSocketController_Message(
		cxHttpStream::Ptr stream,
		int opcode, void* data, unsigned int data_size)
	{
		cxHttpServer::Ptr server = g_servers[stream->m_server_fd];
		if (server == NULL)
		{
			stream->Close();
			return;
		}

		std::shared_ptr<cxWebController> controller = server->m_user_controller[stream->m_url];
		if (controller == NULL)
		{
			controller = server->m_web_controller;
		}
		if (controller == NULL)
		{
			cxHttpResponse::Ptr response = std::make_shared<cxHttpResponse>(stream);
			response->Response(404);
			stream->Close();
			return;
		}

		if (!controller->IsWebSocket())
		{
			cxHttpResponse::Ptr response = std::make_shared<cxHttpResponse>(stream);
			response->Response(400);
			stream->Close();
			return;
		}

		switch (opcode)
		{
		case 0x00: // continuation frame
			break;
		case 0x01: // text frame
			if (data != NULL && data_size > 0)
			{
				controller->WS_OnRecvStr(stream, std::string((char*)data, (char*)data + data_size));
			}
			else
			{
				controller->WS_OnRecvStr(stream, "");
			}
			break;
		case 0x02: // binary frame
			controller->WS_OnRecvBin(stream, data, data_size);
			break;
		case 0x08: // connection close
			controller->WS_OnClose(stream);
			stream->Close();
			return;
		case 0x09: // ping frame
			controller->WS_OnPing(stream);
			break;
		case 0x0A: // pong frame
			controller->WS_OnPong(stream);
			break;
		}
	}
	void cxHttpServer::OnClose(cxSocketTcp::Ptr sock)
	{
		//if (g_streams->find(sock->GetFD()) != g_streams->end())
		if (g_streams.find(sock->GetFD()))
		{
			//g_streams_mutex.lock();
			//(*g_streams)[sock->GetFD()] = NULL;
			//g_streams_mutex.unlock();
			//cxLogerEnable("http客户端[" << sock->GetIp() << ":" << sock->GetPort() << "]连接断开");
		}
		else if (g_servers.find(sock->GetFD()) != g_servers.end())
		{
			g_servers_mutex.lock();
			g_servers[sock->GetFD()] = NULL;
			g_servers_mutex.unlock();
			cxLogerEnable("http服务[" << sock->GetIp() << ":" << sock->GetPort() << "]连接断开，停止服务");
		}
	}
}