#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include <fstream>
#include "../socket/cx_socket.h"
#include "../thread/cx_thread_blockqueue.h"

namespace cx
{
	class cxHttpStream : public std::enable_shared_from_this<cxHttpStream>
	{
	public:
		cxSocketTcp::Ptr											m_socket = NULL;
		SocketFD													m_server_fd = ERROR_SOCKET;
		std::string													m_header_string = "";
		cxSafeList<std::shared_ptr<std::vector<unsigned char>>>		m_recv_buf;
		std::vector<char>											m_ws_temp;
		std::string													m_url = "";
		int															m_temp_opcode = 0;
		bool														m_bws = false;
	public:
		using Ptr = std::shared_ptr<cxHttpStream>;
	public:
		cxHttpStream(cxSocketTcp::Ptr sock, SocketFD server_fd);
		~cxHttpStream();
		std::string GetIp();
		int GetPort();
		SocketFD GetFD();
		SocketFD GetServerFD();
		bool IsConnected();
		void Close();
		void Complete();
		int ParseHeader();
		bool WS_SendData(std::string data);
		bool WS_SendData(void* data, unsigned int data_size);
		bool WS_Ping();
		bool WS_Pong();
	};
}
