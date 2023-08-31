#include "cx_thread_threadpool.h"
#include "../common/cx_common_fun.h"

namespace cx
{
	cxBlockQueue<std::function<void()>>							cxThreadPool::m_threaddatas;
	std::list<std::shared_ptr<cxThreadPool_ThreadInfo>>			cxThreadPool::m_threadinfos;
	bool														cxThreadPool::m_binit = false;
	std::mutex													cxThreadPool::m_init_mutex;

	void cxThreadPool::Init()
	{
		if (m_binit) return;
		std::unique_lock<std::mutex> Lock(m_init_mutex);
		if (m_binit) return;

		int threads = cxCommonFun::GetCpuCount() * 10;

		for (int i = 0; i < threads; i++)
		{
			std::thread td(Thread_Fun);
			td.detach();
		}

		m_binit = true;
	}
	void cxThreadPool::Thread_Fun()
	{
		while (true)
		{
			std::function<void()> task;
			if (!m_threaddatas.wait(task) || task == NULL)
			{
				continue;
			}

			task();
		}
	}
}
