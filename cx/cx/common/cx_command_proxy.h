#pragma once
#include <iostream>
#include <vector>

namespace cx
{
	class cxCommandProxy
	{
	public:
		static void InitConnectInfo(const std::string& ip, int port);
		static int Command(const std::string& command, unsigned int time_out = 3000);
		static int Command(const std::string& command, std::vector<std::string>& lines, unsigned int time_out = 3000);
	};
}