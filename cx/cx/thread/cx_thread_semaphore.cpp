#include "cx_thread_semaphore.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxSemaphore::cxSemaphore(unsigned int init_size, unsigned int max_size)
	{
		if (max_size > 0x7FFFFFFF)
		{
			max_size = 0x7FFFFFFF;
		}

		memset(&m_wait, 0, sizeof(m_wait));
		memset(&m_release, 0, sizeof(m_release));

		sem_init(&m_wait, 0, max_size - init_size);
		sem_init(&m_release, 0, init_size);
	}
	cxSemaphore::~cxSemaphore()
	{
		sem_close(&m_wait);
		sem_close(&m_release);
		sem_destroy(&m_wait);
		sem_destroy(&m_release);
	}
	bool cxSemaphore::WaitOne(unsigned long long millisecond)
	{
		if (millisecond == 0)
		{
			if (sem_wait(&m_release) == 0)
			{
				return sem_post(&m_wait) == 0;
			}
		}
		else
		{
			timespec ts;
#if (__GNUC__ <= 7)
			clock_gettime(CLOCK_REALTIME, &ts);
#else
			clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
			ts.tv_sec += millisecond / 1000;
			ts.tv_nsec += (millisecond % 1000) * 1000000;
			ts.tv_sec += ts.tv_nsec / 1000000000;
			ts.tv_nsec = ts.tv_nsec % 1000000000;
#if (__GNUC__ <= 7)
			if (sem_timedwait(&m_release, &ts) == 0)
#else
			if (sem_clockwait(&m_release, CLOCK_MONOTONIC, &ts) == 0)
#endif
			{
				return sem_post(&m_wait) == 0;
			}
		}

		return false;
	}
	bool cxSemaphore::TryWaitOne()
	{
		if (sem_trywait(&m_release) == 0)
		{
			return sem_post(&m_wait) == 0;
		}
		return false;
	}
	bool cxSemaphore::ReleaseOne(unsigned long long millisecond)
	{
		if (millisecond == 0)
		{
			if (sem_wait(&m_wait) == 0)
			{
				return sem_post(&m_release) == 0;
			}
		}
		else
		{
			timespec ts;
#if (__GNUC__ <= 7)
			clock_gettime(CLOCK_REALTIME, &ts);
#else
			clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

			ts.tv_sec += millisecond / 1000;
			ts.tv_nsec += (millisecond % 1000) * 1000000;
			ts.tv_sec += ts.tv_nsec / 1000000000;
			ts.tv_nsec = ts.tv_nsec % 1000000000;

#if (__GNUC__ <= 7)
			if (sem_timedwait(&m_release, &ts) == 0)
#else
			if (sem_clockwait(&m_wait, CLOCK_MONOTONIC, &ts) == 0)
#endif
			{
				return sem_post(&m_release) == 0;
			}
		}

		return false;
	}
	bool cxSemaphore::TryReleaseOne()
	{
		if (sem_trywait(&m_wait) == 0)
		{
			return sem_post(&m_release) == 0;
		}

		return false;
	}
}
