#pragma once
#include "../socket/cx_socket.h"

// 此方法已过时！！！

namespace cx
{
	enum class WSDATATYPE
	{
		unknow,
		string,
		binary,
		ping,
		pong
	};

	// 此方法已过时！！！
	class cxWebSocketClient : public std::enable_shared_from_this<cxWebSocketClient>
	{
	private:
		cxSocketTcp::Ptr							m_socket = NULL;
		std::string									m_Sec_WebSocket_Key = "";
		std::vector<unsigned char>					m_recv_buf;
		int											m_temp_opcode = 0;
		std::vector<char>							m_ws_temp;
		std::mutex									m_mutex;
	public:
		using Ptr = std::shared_ptr<cxWebSocketClient>;
	public:
		cxWebSocketClient();
		~cxWebSocketClient();
		static Ptr CreatePtr();
		bool IsConnected();
		void Close();
		int ConnectServer(std::string ws_url, unsigned int time_out = 0);
		bool SendData(std::string data);
		bool SendData(void* data, unsigned int data_size);
		bool SendPing();
		bool SendPong();
		bool Recv(WSDATATYPE& type, std::shared_ptr<std::vector<char>>& data, unsigned int time_out = 0, bool auto_ping = true);
	private:
		bool _RecvData(unsigned int time_out);
		bool _ParseRequestUrl(std::string ws_url, std::string& url, std::string& host, int& port);
	};
}