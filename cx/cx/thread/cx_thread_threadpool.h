#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <future>
#include "cx_thread_blockqueue.h"

namespace cx
{
	typedef struct cxThreadPool_ThreadInfo
	{
		bool m_lock = false;
		std::thread td;
		std::mutex m_mutex;
		cxThreadPool_ThreadInfo()
		{

		}
		~cxThreadPool_ThreadInfo()
		{
			m_lock = false;
			m_mutex.lock();
			m_mutex.unlock();
		}
	}cxThreadPool_ThreadInfo;



	/// <summary>
	/// 线程池，全局静态线程池，最小线程数1，最大线程数为cpu线程数的十倍
	/// </summary>
	class cxThreadPool
	{
	private:
		static cxBlockQueue<std::function<void()>>							m_threaddatas;
		static std::list<std::shared_ptr<cxThreadPool_ThreadInfo>>			m_threadinfos;
		static bool															m_binit;
		static std::mutex													m_init_mutex;
	public:
		static void Init();
	public:
		/// <summary>
		/// 运行线程（将线程添加到线程池）
		/// </summary>
		/// <typeparam name="F"></typeparam>
		/// <typeparam name="...Args"></typeparam>
		/// <param name="f"></param>
		/// <param name="...args"></param>
		/// <returns></returns>
		template<class F, class... Args>
		static void RunThread(F&& f, Args&&... args)
		{
			Init();
			std::function<void()> task = std::bind(std::forward<F>(f), args...);
			m_threaddatas.push(task);
		}
		static void RunThread(std::function<void()> task)
		{
			Init();
			m_threaddatas.push(task);
		}
	private:
		static void Thread_Fun();
	};
}
