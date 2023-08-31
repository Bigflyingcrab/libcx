#include "cx_time.h"
#include <sys/time.h>
#include <memory.h>
#include <sstream>
#include <iomanip>
#include "../common/cx_command_proxy.h"
#include "../common/cx_common_fun.h"

namespace cx
{
	cxTimeSpan::cxTimeSpan()
	{

	}
	cxTimeSpan::cxTimeSpan(const cxTimeSpan& ts)
	{
		m_total_millisecond = ts.m_total_millisecond;
	}
	cxTimeSpan& cxTimeSpan::operator=(const cxTimeSpan& ts)
	{
		m_total_millisecond = ts.m_total_millisecond;
		return *this;
	}
	cxTimeSpan cxTimeSpan::operator+(const cxTimeSpan& ts)
	{
		cxTimeSpan ret;
		ret.m_total_millisecond = m_total_millisecond;
		ret.m_total_millisecond += ts.m_total_millisecond;
		return ret;
	}
	cxTimeSpan& cxTimeSpan::operator+=(const cxTimeSpan& ts)
	{
		m_total_millisecond += ts.m_total_millisecond;
		return *this;
	}
	cxTimeSpan cxTimeSpan::operator-(const cxTimeSpan& ts)
	{
		cxTimeSpan ret;
		ret.m_total_millisecond = m_total_millisecond;
		ret.m_total_millisecond -= ts.m_total_millisecond;
		return ret;
	}
	cxTimeSpan& cxTimeSpan::operator-=(const cxTimeSpan& ts)
	{
		m_total_millisecond -= ts.m_total_millisecond;
		return *this;
	}
	bool cxTimeSpan::operator==(const cxTimeSpan& ts)
	{
		return m_total_millisecond == ts.m_total_millisecond;
	}
	bool cxTimeSpan::operator>(const cxTimeSpan& ts)
	{
		return m_total_millisecond > ts.m_total_millisecond;
	}
	bool cxTimeSpan::operator>=(const cxTimeSpan& ts)
	{
		return m_total_millisecond >= ts.m_total_millisecond;
	}
	bool cxTimeSpan::operator<(const cxTimeSpan& ts)
	{
		return m_total_millisecond < ts.m_total_millisecond;
	}
	bool cxTimeSpan::operator<=(const cxTimeSpan& ts)
	{
		return m_total_millisecond <= ts.m_total_millisecond;
	}
	void cxTimeSpan::SetMillisecond(long long millisecond)
	{
		m_total_millisecond = millisecond;
	}
	void cxTimeSpan::SetSecond(long long second)
	{
		m_total_millisecond = second * 1000;
	}
	void cxTimeSpan::SetMinute(long long minute)
	{
		m_total_millisecond = minute * 60000;
	}
	void cxTimeSpan::SetHour(long long hour)
	{
		m_total_millisecond = hour * 3600000;
	}
	void cxTimeSpan::SetDay(long long day)
	{
		m_total_millisecond = day * 86400000;
	}
	long long cxTimeSpan::GetMillisecond()
	{
		return m_total_millisecond;
	}
	long long cxTimeSpan::GetSecond()
	{
		return m_total_millisecond / 1000;
	}
	long long cxTimeSpan::GetMinute()
	{
		return m_total_millisecond / 60000;
	}
	long long cxTimeSpan::GetHour()
	{
		return m_total_millisecond / 3600000;
	}
	long long cxTimeSpan::GetDay()
	{
		return m_total_millisecond / 86400000;
	}

	inline bool CX_LEAPYEAR(int year)
	{
		return (((year) % 400 == 0) || ((year) % 4 == 0 && (year) % 100 != 0));
	}
	inline int CX_YEARDAYS(int year)
	{
		return (CX_LEAPYEAR(year) ? 366 : 365);
	}
	inline int CX_MOTHDAYS(int year, int moth)
	{
		return ((moth) == 1 ? 31 : \
			(moth) == 2 ? (CX_LEAPYEAR(year) ? 29 : 28) : \
			(moth) == 3 ? 31 : \
			(moth) == 4 ? 30 : \
			(moth) == 5 ? 31 : \
			(moth) == 6 ? 30 : \
			(moth) == 7 ? 31 : \
			(moth) == 8 ? 31 : \
			(moth) == 9 ? 30 : \
			(moth) == 10 ? 31 : \
			(moth) == 11 ? 30 : \
			(moth) == 12 ? 31 : 0);
	}

	cxDateTime::cxDateTime()
	{

	}
	cxDateTime::cxDateTime(int year, int moth, int day)
	{
		m_year = year;
		m_moth = moth;
		m_day = day;
		m_hour = 0;
		m_minute = 0;
		m_second = 0;
		m_millisecond = 0;
		SumTime();
	}
	cxDateTime::cxDateTime(int year, int moth, int day, int hour, int minute, int second, int millisecond)
	{
		m_year = year;
		m_moth = moth;
		m_day = day;
		m_hour = hour;
		m_minute = minute;
		m_second = second;
		m_millisecond = millisecond;
		SumTime();
	}
	cxDateTime::cxDateTime(const cxDateTime& dt)
	{
		m_year = dt.m_year;
		m_moth = dt.m_moth;
		m_day = dt.m_day;
		m_hour = dt.m_hour;
		m_minute = dt.m_minute;
		m_second = dt.m_second;
		m_millisecond = dt.m_millisecond;
		m_total_millisecond = dt.m_total_millisecond;
	}
	cxDateTime::~cxDateTime()
	{

	}
	bool cxDateTime::IsNull()
	{
		return m_year == -1 || m_moth == -1 || m_day == -1 || m_hour == -1 || m_minute == -1 || m_second == -1 || m_millisecond == -1 || m_total_millisecond == -1;
	}
	cxDateTime cxDateTime::operator+(cxTimeSpan& ts)
	{
		cxDateTime ret(*this);
		ret.m_total_millisecond += ts.GetMillisecond();
		ret.UpdateTime();
		return ret;
	}
	cxDateTime& cxDateTime::operator+=(cxTimeSpan& ts)
	{
		m_total_millisecond += ts.GetMillisecond();
		UpdateTime();
		return *this;
	}
	cxDateTime cxDateTime::operator-(cxTimeSpan& ts)
	{
		cxDateTime ret(*this);
		ret.m_total_millisecond -= ts.GetMillisecond();
		ret.UpdateTime();
		return ret;
	}
	cxDateTime& cxDateTime::operator-=(cxTimeSpan& ts)
	{
		m_total_millisecond -= ts.GetMillisecond();
		UpdateTime();
		return *this;
	}
	cxTimeSpan cxDateTime::operator-(const cxDateTime& dt)
	{
		cxTimeSpan ts;
		ts.SetMillisecond(m_total_millisecond - dt.m_total_millisecond);
		return ts;
	}
	bool cxDateTime::operator==(const cxDateTime& ts)
	{
		return m_total_millisecond == ts.m_total_millisecond;
	}
	bool cxDateTime::operator>(const cxDateTime& ts)
	{
		return m_total_millisecond > ts.m_total_millisecond;
	}
	bool cxDateTime::operator>=(const cxDateTime& ts)
	{
		return m_total_millisecond >= ts.m_total_millisecond;
	}
	bool cxDateTime::operator<(const cxDateTime& ts)
	{
		return m_total_millisecond < ts.m_total_millisecond;
	}
	bool cxDateTime::operator<=(const cxDateTime& ts)
	{
		return m_total_millisecond <= ts.m_total_millisecond;
	}
	int cxDateTime::GetYear()
	{
		return m_year;
	}
	int cxDateTime::GetMoth()
	{
		return m_moth;
	}
	int cxDateTime::GetDay()
	{
		return m_day;
	}
	int cxDateTime::GetHour()
	{
		return m_hour;
	}
	int cxDateTime::GetMinute()
	{
		return m_minute;
	}
	int cxDateTime::GetSecond()
	{
		return m_second;
	}
	int cxDateTime::GetMillisecond()
	{
		return m_millisecond;
	}
	int cxDateTime::GetWeek()
	{
		if (m_total_millisecond < 0)
		{
			return -1;
		}
		long long week = m_total_millisecond / 86400000;
		week = week % 7;
		return (int)week;
	}
	void cxDateTime::SetYear(int year)
	{
		bool bupdate = m_year != year;
		m_year = year;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetMoth(int moth)
	{
		bool bupdate = m_moth != moth;
		m_moth = moth;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetDay(int day)
	{
		bool bupdate = m_day != day;
		m_day = day;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetHour(int hour)
	{
		bool bupdate = m_hour != hour;
		m_hour = hour;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetMinute(int minute)
	{
		bool bupdate = m_minute != minute;
		m_minute = minute;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetSecond(int second)
	{
		bool bupdate = m_second != second;
		m_second = second;
		if (bupdate)
		{
			SumTime();
		}
	}
	void cxDateTime::SetMillisecond(int millisecond)
	{
		bool bupdate = m_millisecond != millisecond;
		m_millisecond = millisecond;
		if (bupdate)
		{
			SumTime();
		}
	}
	long long cxDateTime::GetTotalMillisecond()
	{
		return m_total_millisecond;
	}
	std::string cxDateTime::toString(std::string format)
	{
		int index = 0;
		std::string ret = format;

		while (true)
		{
			index = ret.find("%YYYY");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(4) << std::setfill('0') << m_year;
				ret = ret.replace(index, strlen("%YYYY"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%YY");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << (m_year % 100);
				ret = ret.replace(index, strlen("%YY"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%MM");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_moth;
				ret = ret.replace(index, strlen("%MM"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%M");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_moth;
				ret = ret.replace(index, strlen("%M"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%DD");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_day;
				ret = ret.replace(index, strlen("%DD"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%D");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_day;
				ret = ret.replace(index, strlen("%D"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%hh");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_hour;
				ret = ret.replace(index, strlen("%hh"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%h");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << (m_hour > 12 ? (m_hour - 12) : m_hour);
				ret = ret.replace(index, strlen("%h"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%HH");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_hour;
				ret = ret.replace(index, strlen("%hh"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%H");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_hour;
				ret = ret.replace(index, strlen("%h"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%mm");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_minute;
				ret = ret.replace(index, strlen("%mm"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%m");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_minute;
				ret = ret.replace(index, strlen("%m"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%ss");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_second;
				ret = ret.replace(index, strlen("%ss"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%s");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_second;
				ret = ret.replace(index, strlen("%s"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%fff");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(3) << std::setfill('0') << m_millisecond;
				ret = ret.replace(index, strlen("%fff"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%ff");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << std::setw(2) << std::setfill('0') << m_millisecond;
				ret = ret.replace(index, strlen("%ff"), ss.str());
			}
			else
			{
				break;
			}
		}

		while (true)
		{
			index = ret.find("%f");
			if (index >= 0)
			{
				std::stringstream ss;
				ss << m_millisecond;
				ret = ret.replace(index, strlen("%f"), ss.str());
			}
			else
			{
				break;
			}
		}

		return ret;
	}
	cxDateTime cxDateTime::Now()
	{
		cxDateTime dt;
		timespec ts;
		memset(&ts, 0, sizeof(timespec));
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 28800;
		tm* t = gmtime(&ts.tv_sec);
		dt.m_year = t->tm_year + 1900;
		dt.m_moth = t->tm_mon + 1;
		dt.m_day = t->tm_mday;
		dt.m_hour = t->tm_hour;
		dt.m_minute = t->tm_min;
		dt.m_second = t->tm_sec;
		dt.m_millisecond = ts.tv_nsec / 1000000;
		dt.SumTime();
		return dt;
	}
	cxDateTime cxDateTime::FromTimespec(struct timespec& ts)
	{
		cxDateTime dt;
		ts.tv_sec += 28800;
		tm* t = gmtime(&ts.tv_sec);
		dt.m_year = t->tm_year + 1900;
		dt.m_moth = t->tm_mon + 1;
		dt.m_day = t->tm_mday;
		dt.m_hour = t->tm_hour;
		dt.m_minute = t->tm_min;
		dt.m_second = t->tm_sec;
		dt.m_millisecond = ts.tv_nsec / 1000000;
		dt.SumTime();
		return dt;
	}
	cxDateTime cxDateTime::FromString(std::string str, std::string format)
	{
		cxDateTime ret = cxDateTime();
		if (str == "" || format == "")
		{
			return ret;
		}

		int year = 0;
		int moth = 0;
		int day = 0;
		int hour = 0;
		int minute = 0;
		int sec = 0;
		int mill = 0;

		int index = 0;
		while (true)
		{
			index = format.find("%");
			if (index < 0)
			{
				break;
			}
			if (format.find("YYYY", index + 1) == index + 1)
			{
				year = cxCommonFun::StringToObject<int>(str.substr(index, 4));
			}
			else if (format.find("MM", index + 1) == index + 1)
			{
				moth = cxCommonFun::StringToObject<int>(str.substr(index, 2));
			}
			else if (format.find("DD", index + 1) == index + 1)
			{
				day = cxCommonFun::StringToObject<int>(str.substr(index, 2));
			}
			else if (format.find("hh", index + 1) == index + 1)
			{
				hour = cxCommonFun::StringToObject<int>(str.substr(index, 2));
			}
			else if (format.find("mm", index + 1) == index + 1)
			{
				minute = cxCommonFun::StringToObject<int>(str.substr(index, 2));
			}
			else if (format.find("ss", index + 1) == index + 1)
			{
				sec = cxCommonFun::StringToObject<int>(str.substr(index, 2));
			}
			format = format.erase(index, 1);
		}

		ret.m_year = year;
		ret.m_moth = moth;
		ret.m_day = day;
		ret.m_hour = hour;
		ret.m_minute = minute;
		ret.m_second = sec;
		ret.m_millisecond = 0;
		ret.SumTime();
		return ret;
	}
	bool cxDateTime::SaveTime2System(cxDateTime dt)
	{
		std::stringstream command;
		command << "date -s '" << dt.toString() << "'";
		return cx::cxCommandProxy::Command(command.str()) > 0;
	}
	long long cxDateTime::GetTick()
	{
		cxDateTime dt(1970, 1, 1, 0, 0, 0);
		cxTimeSpan ts = cxDateTime::Now() - dt;
		return ts.GetMillisecond();
	}
	void cxDateTime::UpdateTime()
	{
		if (m_total_millisecond <= 0)
		{
			Clear();
			return;
		}
		long long year, moth, day, hour, minute, second, millisecond, temp = 0;
		temp = m_total_millisecond;
		millisecond = temp % 1000;
		second = (temp / 1000) % 60;
		minute = (temp / 60000) % 60;
		hour = (temp / 3600000) % 24;
		temp = temp / 86400000;
		year = 1;
		moth = 1;
		day = 1;
		if (temp <= 0)
		{
			return;
		}
		while (temp > CX_YEARDAYS(year))
		{
			year++;
			temp -= CX_YEARDAYS(year);
		}
		while (temp > CX_MOTHDAYS(year, moth))
		{
			moth++;
			temp -= CX_MOTHDAYS(year, moth);
		}
		day = temp;

		m_year = year;
		m_moth = moth;
		m_day = day;
		m_hour = hour;
		m_minute = minute;
		m_second = second;
		m_millisecond = millisecond;
	}
	void cxDateTime::SumTime()
	{
		if (m_year <= 0 || m_year > 9999)
		{
			Clear();
			return;
		}
		if (m_moth <= 0 || m_moth > 12)
		{
			Clear();
			return;
		}
		int temp = CX_MOTHDAYS(m_year, m_moth);
		if (m_day <= 0 || m_day > temp)
		{
			Clear();
			return;
		}
		if (m_hour < 0 || m_hour > 23)
		{
			Clear();
			return;
		}
		if (m_minute < 0 || m_minute > 59)
		{
			Clear();
			return;
		}
		if (m_second < 0 || m_second > 59)
		{
			Clear();
			return;
		}
		if (m_millisecond < 0 || m_millisecond > 999)
		{
			Clear();
			return;
		}

		for (unsigned int i = 1; i < m_year; i++)
		{
			if (CX_LEAPYEAR(i))
			{
				m_total_millisecond += (long long)31622400000;
			}
			else
			{
				m_total_millisecond += (long long)31536000000;
			}
		}

		for (unsigned int i = 1; i < m_moth; i++)
		{
			m_total_millisecond += (unsigned long long)CX_MOTHDAYS(m_year, i) * (unsigned long long)86400000;
		}

		m_total_millisecond += ((unsigned long long)m_day - 1) * (unsigned long long)86400000;
		m_total_millisecond += (unsigned long long)m_hour * (unsigned long long)3600000;
		m_total_millisecond += (unsigned long long)m_minute * (unsigned long long)60000;
		m_total_millisecond += (unsigned long long)m_second * (unsigned long long)1000;
		m_total_millisecond += m_millisecond;
	}
	void cxDateTime::Clear()
	{
		m_year = -1;
		m_moth = -1;
		m_day = -1;
		m_hour = -1;
		m_minute = -1;
		m_second = -1;
		m_millisecond = -1;
		m_total_millisecond = -1;
	}
}

