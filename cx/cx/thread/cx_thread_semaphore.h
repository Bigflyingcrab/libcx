#pragma once
#include <semaphore.h>
#include <memory>
#include <memory.h>

namespace cx
{
	class cxSemaphore
	{
	public:
		using Ptr = std::shared_ptr<cxSemaphore>;
	private:
		sem_t								m_wait;
		sem_t								m_release;
	public:
		cxSemaphore(unsigned int init_size = 0, unsigned int max_size = 0x7FFFFFFF);
		~cxSemaphore();
		bool WaitOne(unsigned long long millisecond = 0);
		bool TryWaitOne();
		bool ReleaseOne(unsigned long long millisecond = 0);
		bool TryReleaseOne();
	};
}
