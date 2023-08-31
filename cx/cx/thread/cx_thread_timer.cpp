#include "cx_thread_timer.h"
#include "../common/cx_common_fun.h"

namespace cx
{
	cxSemaphore::Ptr							cxTimer::m_sem = std::make_shared<cxSemaphore>();
	std::list<cxTimerData>						cxTimer::m_timer_list;
	std::mutex									cxTimer::m_mutex;
	bool										cxTimer::m_binit = false;
	std::mutex									cxTimer::m_init_mutex;


	void cxTimer::TimerBLL_Thread()
	{
		while (true)
		{
			m_mutex.lock();

			if (m_timer_list.empty())
			{
				m_mutex.unlock();
				m_sem->WaitOne();
				continue;
			}
			else
			{
				cxTimerData ptr = m_timer_list.front();
				unsigned long long tick = cxCommonFun::GetTickTime();
				if (ptr.runtime <= tick)
				{
					cxThreadPool::RunThread(ptr.fun);
					m_timer_list.pop_front();
					m_mutex.unlock();
				}
				else
				{
					m_mutex.unlock();
					m_sem->WaitOne(ptr.runtime - tick);
				}
			}
		}
	}

	void cxTimer::AddToList(cxTimerData& ptr)
	{
		m_mutex.lock();
		m_timer_list.push_back(ptr);
		m_timer_list.sort();
		m_mutex.unlock();
		m_sem->ReleaseOne();
	}
}
