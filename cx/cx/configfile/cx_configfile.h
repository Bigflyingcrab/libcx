#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include "../common/cx_common_fun.h"

namespace cx
{
	class cxConfigFile
	{
	private:
		std::map<std::string, std::string>					m_list;
		std::mutex											m_mutex;
		std::string											m_file_path;
	public:
		cxConfigFile();
		~cxConfigFile();
		bool LoadConfig(const std::string& config_file_path);
		bool IsHaveConfig(const std::string& key);
		template<typename T> T GetValue(const std::string& key)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return cxCommonFun::StringToObject<T>(m_list[key]);
		}
	};
}
