#pragma once
#include <iostream>
#include <memory>

namespace cx
{
	/// <summary>
	/// 时间差
	/// </summary>
	class cxTimeSpan
	{
	public:
		using Ptr = std::shared_ptr<cxTimeSpan>;
	private:
		long long					m_total_millisecond = 0;
	public:
		cxTimeSpan();
		cxTimeSpan(const cxTimeSpan& ts);
		cxTimeSpan& operator=(const cxTimeSpan& ts);
		cxTimeSpan operator+(const cxTimeSpan& ts);
		cxTimeSpan& operator+=(const cxTimeSpan& ts);
		cxTimeSpan operator-(const cxTimeSpan& ts);
		cxTimeSpan& operator-=(const cxTimeSpan& ts);
		bool operator==(const cxTimeSpan& ts);
		bool operator>(const cxTimeSpan& ts);
		bool operator>=(const cxTimeSpan& ts);
		bool operator<(const cxTimeSpan& ts);
		bool operator<=(const cxTimeSpan& ts);
		/// <summary>
		/// 设置时间差，以毫秒方式
		/// </summary>
		/// <param name="millisecond"></param>
		void SetMillisecond(long long millisecond);
		/// <summary>
		/// 设置时间差，以秒方式
		/// </summary>
		/// <param name="second"></param>
		void SetSecond(long long second);
		/// <summary>
		/// 设置时间差，以分钟方式
		/// </summary>
		/// <param name="minute"></param>
		void SetMinute(long long minute);
		/// <summary>
		/// 设置时间差，以小时方式
		/// </summary>
		/// <param name="hour"></param>
		void SetHour(long long hour);
		/// <summary>
		/// 设置时间差，以天方式
		/// </summary>
		/// <param name="day"></param>
		void SetDay(long long day);
		/// <summary>
		/// 获取时间差，以毫秒方式
		/// </summary>
		/// <returns></returns>
		long long GetMillisecond();
		/// <summary>
		/// 获取时间差，以秒方式
		/// </summary>
		/// <returns></returns>
		long long GetSecond();
		/// <summary>
		/// 获取时间差，以分钟方式
		/// </summary>
		/// <returns></returns>
		long long GetMinute();
		/// <summary>
		/// 获取时间差，以小时方式
		/// </summary>
		/// <returns></returns>
		long long GetHour();
		/// <summary>
		/// 获取时间差，以天方式
		/// </summary>
		/// <returns></returns>
		long long GetDay();
	};

	/// <summary>
	/// 时间类
	/// </summary>
	class cxDateTime
	{
	public:
		using Ptr = std::shared_ptr<cxDateTime>;
	private:
		int							m_year = -1;
		int							m_moth = -1;
		int							m_day = -1;
		int							m_hour = -1;
		int							m_minute = -1;
		int							m_second = -1;
		int							m_millisecond = -1;
		long long					m_total_millisecond = -1;
	public:
		cxDateTime();
		cxDateTime(int year, int moth, int day);
		cxDateTime(int year, int moth, int day, int hour, int minute, int second, int millisecond = 0);
		cxDateTime(const cxDateTime& dt);
		~cxDateTime();
		bool IsNull();
		cxDateTime operator+(cxTimeSpan& ts);
		cxDateTime& operator+=(cxTimeSpan& ts);
		cxDateTime operator-(cxTimeSpan& ts);
		cxDateTime& operator-=(cxTimeSpan& ts);
		cxTimeSpan operator-(const cxDateTime& dt);
		bool operator==(const cxDateTime& ts);
		bool operator>(const cxDateTime& ts);
		bool operator>=(const cxDateTime& ts);
		bool operator<(const cxDateTime& ts);
		bool operator<=(const cxDateTime& ts);
		int GetYear();
		int GetMoth();
		int GetDay();
		int GetHour();
		int GetMinute();
		int GetSecond();
		int GetMillisecond();
		int GetWeek();
		void SetYear(int year);
		void SetMoth(int moth);
		void SetDay(int day);
		void SetHour(int hour);
		void SetMinute(int minute);
		void SetSecond(int second);
		void SetMillisecond(int millisecond);
		long long GetTotalMillisecond();
		/// <summary>
		/// %YYYY  四位数年
		/// %YY    两位数年
		/// %MM    两位数月，小于10补0
		/// %M     一位数月
		/// %DD    两位数日，小于10补0
		/// %D     一位数日
		/// %hh    两位数12小时制的时，小于10补0
		/// %h     一位数12小时制的时
		/// %HH    两位数24小时制的时，小于10补0
		/// %H     一位数24小时制的时
		/// %mm    两位数分，小于10补0
		/// %m     一位数分
		/// %ss    两位数秒，小于10补0
		/// %s     一位数秒
		/// </summary>
		/// <param name="format"></param>
		/// <returns></returns>
		std::string toString(std::string format = "%YYYY-%MM-%DD %hh:%mm:%ss");		
	public:
		static cxDateTime Now();
		static cxDateTime FromTimespec(struct timespec& ts);
		static cxDateTime FromString(std::string str_dt, std::string format = "%YYYY-%MM-%DD %hh:%mm:%ss");
		static bool SaveTime2System(cxDateTime dt);
		static long long GetTick();
	private:
		void UpdateTime();
		void SumTime();
		void Clear();
	};
}