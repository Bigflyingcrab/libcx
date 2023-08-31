#pragma once
#include <memory.h>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../common/cx_common_fun.h"

#define									ERROR_SOCKET								-1
#define									RECVBUFSIZE									8192

namespace cx
{
	typedef int SocketFD;

	typedef union __NetHost
	{
		char c[2];
		short s;
	}__NetHost;

	/// <summary>
	/// 判断当前系统是否为小端系统
	/// </summary>
	/// <returns></returns>
	inline bool LittleSystem()
	{
		__NetHost nh;
		nh.s = 1;
		return nh.c[0] == 1;
	}
	/// <summary>
	/// 主机序转网络序
	/// </summary>
	/// <param name="p"></param>
	/// <param name="in"></param>
	inline void Host2Net(unsigned char* p, unsigned short in)
	{
		if (LittleSystem())
		{
			p[0] = ((unsigned char*)&in)[1];
			p[1] = ((unsigned char*)&in)[0];
		}
		else
		{
			memcpy(p, &in, sizeof(in));
		}
	}
	/// <summary>
	/// 主机序转网络序
	/// </summary>
	/// <param name="p"></param>
	/// <param name="in"></param>
	inline void Host2Net(unsigned char* p, unsigned int in)
	{
		if (LittleSystem())
		{
			p[0] = ((unsigned char*)&in)[3];
			p[1] = ((unsigned char*)&in)[2];
			p[2] = ((unsigned char*)&in)[1];
			p[3] = ((unsigned char*)&in)[0];
		}
		else
		{
			memcpy(p, &in, sizeof(in));
		}
	}
	/// <summary>
	/// 主机序转网络序
	/// </summary>
	/// <param name="p"></param>
	/// <param name="in"></param>
	inline void Host2Net(unsigned char* p, unsigned long long in)
	{
		if (LittleSystem())
		{
			p[0] = ((unsigned char*)&in)[7];
			p[1] = ((unsigned char*)&in)[6];
			p[2] = ((unsigned char*)&in)[5];
			p[3] = ((unsigned char*)&in)[4];
			p[4] = ((unsigned char*)&in)[3];
			p[5] = ((unsigned char*)&in)[2];
			p[6] = ((unsigned char*)&in)[1];
			p[7] = ((unsigned char*)&in)[0];
		}
		else
		{
			memcpy(p, &in, sizeof(in));
		}
	}
	/// <summary>
	/// 网络字节序转本机整形
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	inline unsigned short Net2Host16(unsigned char* p)
	{
		unsigned short ret = 0;
		if (LittleSystem())
		{
			ret |= ((unsigned short)p[1] & 0xFF);
			ret |= (((unsigned short)p[0] << 8) & 0xFF00);
		}
		else
		{
			ret = *(unsigned short*)p;
		}
		return ret;
	}
	/// <summary>
	/// 网络字节序转本机整形
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	inline unsigned int Net2Host32(unsigned char* p)
	{
		unsigned int ret = 0;
		if (LittleSystem())
		{
			ret |= ((unsigned int)p[3] & 0xFF);
			ret |= (((unsigned int)p[2] << 8) & 0xFF00);
			ret |= (((unsigned int)p[1] << 16) & 0xFF0000);
			ret |= (((unsigned int)p[0] << 24) & 0xFF000000);
		}
		else
		{
			ret = *(unsigned int*)p;
		}
		return ret;
	}
	/// <summary>
	/// 网络字节序转本机整形
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	inline unsigned long long Net2Host64(unsigned char* p)
	{
		unsigned long long ret = 0;
		if (LittleSystem())
		{
			ret |= ((unsigned long long)p[7] & 0xFF);
			ret |= (((unsigned long long)p[6] << 8) & 0xFF00);
			ret |= (((unsigned long long)p[5] << 16) & 0xFF0000);
			ret |= (((unsigned long long)p[4] << 24) & 0xFF000000);
			ret |= (((unsigned long long)p[3] << 32) & 0xFF00000000);
			ret |= (((unsigned long long)p[2] << 40) & 0xFF0000000000);
			ret |= (((unsigned long long)p[1] << 48) & 0xFF000000000000);
			ret |= (((unsigned long long)p[0] << 56) & 0xFF00000000000000);
		}
		else
		{
			ret = *(unsigned long long*)p;
		}
		return ret;
	}
	inline bool IsIpStr(std::string str)
	{
		std::vector<std::string> group;
		cxCommonFun::StringToGroup(str, ".", group);
		if (group.size() != 4)
		{
			return false;
		}

		for (unsigned int i = 0; i < group.size(); i++)
		{
			for (unsigned int j = 0; j < group[i].length(); j++)
			{
				if (group[i][j] < '0' || group[i][j] > '9')
				{
					return false;
				}
			}

			int num = cxCommonFun::StringToObject<int>(group[i]);
			if (num < 0 || num > 255)
			{
				return false;
			}
		}

		return true;

		/*std::string temp = "";
		for (unsigned int i = 0; i < str.length(); i++)
		{
			if (str[i] == '.')
			{
				ip.push_back(temp);
				temp = "";
				continue;
			}
			temp += str[i];
		}
		if (ip.size() != 4)
		{
			return false;
		}
		for (unsigned int i = 0; i < ip.size(); i++)
		{
			if (!cxCommonFun::StringIsNum(ip[i]))
			{
				return false;
			}
		}
		return true;*/
	}
	inline std::string GetHostByName(std::string name)
	{
		if (IsIpStr(name))
		{
			return name;
		}

		struct hostent* host = gethostbyname(name.c_str());
		if (host == NULL || host->h_addr_list == NULL)
		{
			return "";
		}

		return std::string(inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
	}
}
