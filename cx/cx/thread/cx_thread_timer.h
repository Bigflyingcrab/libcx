#pragma once
#include <mutex>
#include <thread>
#include <memory>
#include <functional>
#include <list>
#include "cx_thread_threadpool.h"
#include "cx_thread_semaphore.h"
#include "../common/cx_common_fun.h"

namespace cx
{
	typedef struct cxTimerData
	{
		std::function<void()> fun = NULL;
		unsigned long long runtime = 0;
		cxTimerData()
		{

		}
		cxTimerData(const cxTimerData& td)
		{
			fun = td.fun;
			runtime = td.runtime;
		}
		~cxTimerData()
		{

		}
		bool operator>(const cxTimerData& td)
		{
			return runtime > td.runtime;
		}
		bool operator>=(const cxTimerData& td)
		{
			return runtime >= td.runtime;
		}
		bool operator==(const cxTimerData& td)
		{
			return runtime == td.runtime;
		}
		bool operator<(const cxTimerData& td)
		{
			return runtime < td.runtime;
		}
		bool operator<=(const cxTimerData& td)
		{
			return runtime <= td.runtime;
		}
	}cxTimerData;

	class cxTimer
	{
	private:
		static cxSemaphore::Ptr								m_sem;
		static std::list<cxTimerData>						m_timer_list;
		static std::mutex									m_mutex;
		static bool											m_binit;
		static std::mutex									m_init_mutex;
	private:
		static void TimerBLL_Thread();
		static void AddToList(cxTimerData& ptr);
	public:
		template<class F, class... Args>
		static void AddTimer(unsigned long long interval, F&& f, Args&&... args)
		{
			if (!m_binit)
			{
				m_init_mutex.lock();
				if (!m_binit)
				{
					std::thread td(TimerBLL_Thread);
					td.detach();
					m_binit = true;
				}
				m_init_mutex.unlock();
			}

			cxTimerData ptr;
			ptr.fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
			ptr.runtime = cxCommonFun::GetTickTime() + interval;

			AddToList(ptr);
		}
	};
}
