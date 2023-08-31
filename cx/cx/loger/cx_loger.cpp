#include "cx_loger.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "../common/cx_common_fun.h"
#include <unistd.h>
#include "../thread/cx_thread_timer.h"
#include <iomanip>
#include <fstream>
#include "../common/cx_common_fun.h"

namespace cx
{
	cxBlockQueue<cxLoger::cxLogerData>							cxLoger::m_log_data;
	cxBlockQueue<std::string>									cxLoger::m_write_list;
	bool														cxLoger::m_binit = false;
	std::mutex													cxLoger::m_init_mutex;
	std::string													cxLoger::m_savePath = cxCommonFun::GetAppPath() + "/Log/";
	int															cxLoger::m_max_saveDays = 30;
	int															cxLoger::m_level = 0x1F;
	bool														cxLoger::m_lock_work = false;
	bool														cxLoger::m_lock_write = false;
	std::mutex													cxLoger::m_mutex_work;
	std::mutex													cxLoger::m_mutex_write;


	void cxLoger::OutPut(const cxLogLevel& level, const std::string& filename, const std::string& funname, const int& line, StringStream& log)
	{	
		cxLogerData ld;
		ld.time = cxDateTime::Now();
		ld.level = level;
#if (__GNUC__ <= 7)
		ld.thread_id = syscall(SYS_gettid);
#else
		ld.thread_id = gettid();
#endif		
		ld.line = line;
		ld.fun_name = funname;
		ld.file_name = filename;
		ld.log = log.str();
		m_log_data.push(ld);

		if (!m_binit)
		{
			Init();
		}
	}
	void cxLoger::SetLevel(int level)
	{
		m_level = level;

		if (!m_binit)
		{
			Init();
		}
	}
	void cxLoger::SetPath(std::string path)
	{
		m_savePath = path;
		if (m_savePath == "")
		{
			return;
		}

		if (m_savePath[m_savePath.size() - 1] != '/')
		{
			m_savePath += '/';
		}

		if (!m_binit)
		{
			Init();
		}
	}
	std::string cxLoger::GetPath()
	{
		return m_savePath;
	}
	void cxLoger::SetMaxSaveDays(int days)
	{
		m_max_saveDays = days;

		if (!m_binit)
		{
			Init();
		}
	}
	void cxLoger::Stop()
	{
		if (!m_binit)
		{
			return;
		}
		std::unique_lock<std::mutex> Lock(m_init_mutex);
		if (!m_binit)
		{
			return;
		}

		m_lock_work = false;
		m_lock_write = false;

		m_mutex_work.lock();
		m_mutex_work.unlock();
		m_mutex_write.lock();
		m_mutex_write.unlock();

		std::string str = "";
		while (!m_log_data.empty())
		{
			cxLogerData ld;
			if (m_log_data.try_wait(ld))
			{
				str += GetLogStr(ld);
			}
		}

		if (str != "")
		{
			std::cout << str;
			WriteFile(str);
		}
	}

	void cxLoger::Init()
	{
		if (m_binit)
		{
			return;
		}
		std::unique_lock<std::mutex> Lock(m_init_mutex);
		if (m_binit)
		{
			return;
		}

		m_lock_work = true;
		m_lock_write = true;

		std::thread td_work(Thread_Work);
		td_work.detach();

		std::thread td_write(Thread_Write);
		td_write.detach();

		m_binit = true;
	}
	void cxLoger::Thread_Work()
	{
		std::unique_lock<std::mutex> Lock(m_mutex_work);

		unsigned long long tick_console = cxCommonFun::GetTickTime();
		unsigned long long tick_file = tick_console;
		std::string str_console = "";
		std::string str_file = "";
		while (m_lock_work)
		{
			cxLogerData ld;
			if (m_log_data.wait(ld, 200))
			{
				std::string str = GetLogStr(ld);
				str_console += str;
				str_file += str;
			}
			
			unsigned long long tick = cxCommonFun::GetTickTime();

			if (tick - tick_console >= 200 && str_console != "")
			{
				std::cout << str_console;
				str_console = "";
				tick_console = tick;
			}

			if (tick - tick_file >= 3000 && str_file != "")
			{
				m_write_list.push(str_file);
				str_file = "";
				tick_file = tick;
			}
		}

		if (str_console != "")
		{
			std::cout << str_console;
		}

		if (str_file != "")
		{
			m_write_list.push(str_file);
		}
	}
	void cxLoger::Thread_Write()
	{
		std::unique_lock<std::mutex> Lock(m_mutex_write);

		while (m_lock_write)
		{
			std::string log = "";
			if (!m_write_list.wait(log, 500))
			{
				continue;
			}

			WriteFile(log);
		}
	}
	std::string cxLoger::GetLogStr(const cxLogerData& ld)
	{
		if ((m_level & (int)ld.level) != (int)ld.level)
		{
			return "";
		}

		std::stringstream ss;

		switch (ld.level)
		{
		case cxLogLevel::Enable:
			ss << "[Enable]";
			break;
		case cxLogLevel::Error:
			ss << "[Error]";
			break;
		case cxLogLevel::Warning:
			ss << "[Warning]";
			break;
		case cxLogLevel::Exception:
			ss << "[Exception]";
			break;		
		case cxLogLevel::Debug:
			ss << "[Debug]";
			break;
		}

		int ifind = ld.file_name.find_last_of("/");
		std::string file_name = "";
		if (ifind >= 0)
		{
			file_name = ld.file_name.substr(ifind + 1);
		}
		else
		{
			file_name = ld.file_name;
		}

		cxDateTime time = ld.time;

		ss << "[" << time.toString("%YYYY-%MM-%DD") << " " << time.toString("%hh:%mm:%ss.%fff") << "]";
		//ss << "[" << ld.fun_name << "]";
		ss << "[" << file_name << ":" << ld.line << "]";
		ss << "[tid:" << ld.thread_id << "]";
		ss << ": " << ld.log;
		ss << std::endl;

		return ss.str();
	}

	bool cxLoger::WriteFile(std::string& log)
	{
		if (m_savePath == "")
		{
			m_savePath = "./Log/";
		}

		// 获取路径
		std::string path = cx::cxCommonFun::GetFullPath(m_savePath);
		if (path == "")
		{
			return false;
		}

		if (path[path.size() - 1] != '/')
		{
			path += '/';
		}

		cxDateTime now = cxDateTime::Now();		

		if (!cx::cxCommonFun::CreateDirectory(path))
		{
			return false;
		}

		std::stringstream file_path;
		file_path << path << now.GetYear() << "-" << now.GetMoth() << "-" << now.GetDay() << ".log";

		std::ofstream file(file_path.str(), std::ios::app);
		if (file)
		{
			file << log;
			file.close();

			// 删除指定日期之前的日志文件
			cxTimeSpan ts;
			ts.SetDay(m_max_saveDays);
			now -= ts;
			std::vector<cxCommonFun::FileInfo> files;
			cxCommonFun::GetFiles(path, files);
			for (unsigned int i = 0; i < files.size(); i++)
			{
				if (files[i].createtime <= now && files[i].ext_name == "log")
				{
					cxCommonFun::RemoveFile(path + files[i].name + "." + files[i].ext_name);
				}
			}
			return true;
		}

		return false;
	}
}
