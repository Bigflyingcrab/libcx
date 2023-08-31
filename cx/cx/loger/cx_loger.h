#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <mutex>
#include "../thread/cx_thread.h"
#include "../thread/cx_thread_blockqueue.h"
#include "../time/cx_time.h"

namespace cx
{
	typedef enum class cxLogLevel
	{
		Enable = 1,
		Error = 2,
		Warning = 4,
		Exception = 8,
		Debug = 16
	};

	class StringStream
	{
	private:
		std::stringstream					m_ss;
	public:
		StringStream()
		{

		}
		StringStream(const StringStream& stream)
		{
			m_ss << stream.m_ss.str();
		}
		~StringStream()
		{

		}
		void operator=(const StringStream& stream)
		{
			m_ss << stream.m_ss.str();
		}
		StringStream& operator<<(std::string str)
		{
			m_ss << str;
			return *this;
		}
		StringStream& operator<<(const char* str)
		{
			m_ss << str;
			return *this;
		}
		StringStream& operator<<(char c)
		{
			m_ss << c;
			return *this;
		}
		StringStream& operator<<(unsigned char c)
		{
			m_ss << c;
			return *this;
		}
		StringStream& operator<<(short s)
		{
			m_ss << s;
			return *this;
		}
		StringStream& operator<<(unsigned short s)
		{
			m_ss << s;
			return *this;
		}
		StringStream& operator<<(int i)
		{
			m_ss << i;
			return *this;
		}
		StringStream& operator<<(unsigned int i)
		{
			m_ss << i;
			return *this;
		}
		StringStream& operator<<(long l)
		{
			m_ss << l;
			return *this;
		}
		StringStream& operator<<(unsigned long l)
		{
			m_ss << l;
			return *this;
		}
		StringStream& operator<<(long long l)
		{
			m_ss << l;
			return *this;
		}
		StringStream& operator<<(unsigned long long l)
		{
			m_ss << l;
			return *this;
		}
		StringStream& operator<<(float f)
		{
			m_ss << f;
			return *this;
		}
		StringStream& operator<<(double d)
		{
			m_ss << d;
			return *this;
		}
		StringStream& operator<<(bool b)
		{
			if (b)
			{
				m_ss << "true";
			}
			else
			{
				m_ss << "false";
			}
			return *this;
		}
		std::string str()
		{
			return m_ss.str();
		}
	};

	class cxLoger
	{
	private:		
		typedef struct cxLogerData
		{
			unsigned long long				thread_id = 0;
			cxLogLevel						level;
			cxDateTime						time;
			int								line = 0;
			std::string						fun_name = "";
			std::string						file_name = "";
			std::string						log = "";
		}cxLogerData;
	private:
		static cxBlockQueue<cxLogerData>							m_log_data;
		static cxBlockQueue<std::string>							m_write_list;
		static bool													m_binit;
		static std::mutex											m_init_mutex;
		static std::string											m_savePath;
		static int													m_max_saveDays;
		static int													m_level;
		static bool													m_lock_work;
		static bool													m_lock_write;
		static std::mutex											m_mutex_work;
		static std::mutex											m_mutex_write;
	public:
		static void OutPut(const cxLogLevel& level, const std::string& filename, const std::string& funname, const int& line, StringStream& log);
		static void SetLevel(int level);
		static void SetPath(std::string path);
		static std::string GetPath();
		static void SetMaxSaveDays(int days);
		static void Stop();
	private:
		static void Init();
		static void Thread_Work();
		static void Thread_Write();
		static std::string GetLogStr(const cxLogerData& ld);
		static bool WriteFile(std::string& log);
	};

#define cxLogerEnable(STR) (cx::cxLoger::OutPut(cx::cxLogLevel::Enable, __FILE__, __FUNCTION__, __LINE__, cx::StringStream() << STR))
#define cxLogerError(STR) (cx::cxLoger::OutPut(cx::cxLogLevel::Error, __FILE__, __FUNCTION__, __LINE__, cx::StringStream() << STR))
#define cxLogerWarning(STR) (cx::cxLoger::OutPut(cx::cxLogLevel::Warning, __FILE__, __FUNCTION__, __LINE__, cx::StringStream() << STR))
#define cxLogerException(STR) (cx::cxLoger::OutPut(cx::cxLogLevel::Exception, __FILE__, __FUNCTION__, __LINE__, cx::StringStream() << STR))
#define cxLogerDebug(STR) (cx::cxLoger::OutPut(cx::cxLogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, cx::StringStream() << STR))
}
