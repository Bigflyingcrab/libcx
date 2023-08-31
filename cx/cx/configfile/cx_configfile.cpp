#include "cx_configfile.h"
#include <fstream>

namespace cx
{
	cxConfigFile::cxConfigFile()
	{

	}
	cxConfigFile::~cxConfigFile()
	{

	}
	bool IsNotes(std::string value)
	{
		for (unsigned int i = 0; i < value.length(); i++)
		{
			if (value[i] == ' ')
			{
				continue;
			}
			if (value[i] == '#')
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		return false;
	}
	bool cxConfigFile::LoadConfig(const std::string& config_file_path)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		m_list.clear();

		std::ifstream file(config_file_path);
		if (file.is_open())
		{
			std::vector<std::string> lines;
			while (file)
			{
				std::string line;
				std::getline(file, line);
				lines.push_back(line);
			}
			file.close();

			for (unsigned int i = 0; i < lines.size(); i++)
			{
				if (IsNotes(lines[i]))
				{
					continue;
				}
				std::string temp = lines[i];
				bool bkey = true;
				std::string key = "";
				std::string value = "";
				for (unsigned int j = 0; j < temp.length(); j++)
				{
					if (temp[j] == ' ')
					{
						continue;
					}

					if (temp[j] == '=' && bkey)
					{
						bkey = false;
						continue;
					}

					if (temp[j] == '\r' || temp[j] == '\n')
					{
						break;
					}

					if (bkey)
					{
						key += temp[j];
					}
					else
					{
						value += temp[j];
					}
				}

				if (key != "" && value != "")
				{
					m_list[key] = value;
				}
			}
		}

		return true;
	}
	bool cxConfigFile::IsHaveConfig(const std::string& key)
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		return m_list.find(key) != m_list.end();
	}
}
