#pragma once
#include "../socket/cx_socket.h"

namespace cx
{
	class cxWebSocket : public std::enable_shared_from_this<cxWebSocket>
	{
	public:
		using Ptr = std::shared_ptr<cxWebSocket>;
		using On_Accept = std::function<void(cxWebSocket::Ptr server, cxWebSocket::Ptr client)>;
		using On_Message_Str = std::function<void(cxWebSocket::Ptr client, std::string str)>;
		using On_Message_Bin = std::function<void(cxWebSocket::Ptr client, void* data, int len)>;
		using On_Close = std::function<void(cxWebSocket::Ptr client)>;
	private:
		cxSocketTcp::Ptr							m_socket = NULL;
		std::shared_ptr<cxWebSocket>				m_server = NULL;
		bool										m_connected = false;
		std::string									m_url = "";
		std::string									m_Sec_WebSocket_Key = "";
		std::vector<unsigned char>					m_recv_buf;
		int											m_temp_opcode = 0;
		std::vector<char>							m_ws_temp;
		std::mutex									m_mutex;
	private:
		On_Accept									m_cb_accept = NULL;
		On_Message_Str								m_cb_message_str = NULL;
		On_Message_Bin								m_cb_message_bin = NULL;
		On_Close									m_cb_close = NULL;
	private:
		std::vector<char>							m_opcode_buf;
		int											m_opcode_type = 0;
	public:
		cxWebSocket(cxSocketTcp::Ptr sock = NULL);
		~cxWebSocket();
		std::string GetIp();
		int GetPort();
		SocketFD GetFD();
		bool IsConnected();
		void Close();
		int Connect(std::string url, On_Message_Str cb_str, On_Message_Bin cb_bin, On_Close cb_close, long long time_out = 3000);
		bool StartListen(std::string ip, int port, std::string url, On_Accept cb);
		bool Send(void* data, int len);
		bool Send(std::string str);
		bool Send(Json::Value json);
		void SetOnMessageStr(On_Message_Str cb);
		void SetOnMessageBin(On_Message_Bin cb);
		void SetOnClose(On_Close cb);
	private:
		bool _Ping();
		void _Close();
		bool _Send(void* data, int len, bool bstr, bool bclient);
		int _ParseWSData();
		int _ParseWSData_Http();
		int _ParseWSData_WS();
		void _WSMessage(int op, void* data, int len);
		void _WSMessage_String(void* data, int len);
		void _WSMessage_Bin(void* data, int len);
		void _WSMessage_Ping();
		void _WSMessage_Pong();
	private:
		static cxSafeMap<SocketFD, std::shared_ptr<cxWebSocket>>		g_servers;
		static cxSafeMap<SocketFD, std::shared_ptr<cxWebSocket>>		g_clients;
	private:
		static void OnAccept(cxSocketTcp::Ptr server, cxSocketTcp::Ptr client);
		static void OnRecv(cxSocketTcp::Ptr client, void* data, unsigned int size);
		static void OnClose(cxSocketTcp::Ptr client);
		static bool ParseRequestUrl(std::string ws_url, std::string& url, std::string& host, int& port);
	private:
		static bool														g_init;
		static std::mutex												g_init_mutex;
		static void Init();
		static void Thread_Heart();
	};
}
