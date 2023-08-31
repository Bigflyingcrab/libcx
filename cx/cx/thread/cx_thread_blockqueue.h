#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <exception>
#include "cx_thread_semaphore.h"
#include "cx_thread_container.h"

namespace cx
{
	/// <summary>
	/// 线程安全阻塞队列
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T> class cxBlockQueue
	{
	public:
		using Ptr = std::shared_ptr<cxBlockQueue<T>>;
	private:
		cxSafeQueue<T>							m_queue;
		cxSemaphore::Ptr						m_sem;
	public:
		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="max_size">队列的最大容量</param>
		cxBlockQueue(unsigned int max_size = 0x7FFFFFFF)
		{
			m_sem = std::make_shared<cxSemaphore>(0, max_size);
		}
		~cxBlockQueue()
		{

		}
		/// <summary>
		/// 队列是否为空
		/// </summary>
		/// <returns></returns>
		bool empty()
		{
			return m_queue.empty();
		}
		/// <summary>
		/// 获取队列中数据的数量
		/// </summary>
		/// <returns></returns>
		unsigned int size()
		{
			return m_queue.size();
		}
		/// <summary>
		/// 向队列末尾添加一个数据，如果此时队列中数据数量已达到最大值，会根据指定的超时时间进行等待
		/// </summary>
		/// <param name="t">数据</param>
		/// <param name="millisecond">超时时间</param>
		/// <returns>是否添加成功</returns>
		bool push(const T& t, unsigned int millisecond = 0)
		{
			if (m_sem->ReleaseOne(millisecond))
			{
				m_queue.push(t);
				return true;
			}
			return false;
		}
		/// <summary>
		/// 尝试向队列末尾添加一个数据，没有等待时间，不管成功与否都会立即返回
		/// </summary>
		/// <param name="t">数据</param>
		/// <returns>是否添加成功</returns>
		bool try_push(const T& t)
		{
			if (m_sem->TryReleaseOne())
			{
				m_queue.push(t);
				return true;
			}
			return false;
		}
		/// <summary>
		/// 从队列头等待一个数据，成功后此数据也会从队列中出栈
		/// </summary>
		/// <param name="t">数据</param>
		/// <param name="millisecond">超时时间</param>
		/// <returns>是否成功拿到数据</returns>
		bool wait(T& t, unsigned int millisecond = 0)
		{
			if (m_sem->WaitOne(millisecond))
			{
				while (!m_queue.try_pop_front(t))
				{
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				}
				return true;
			}
			return false;
		}
		/// <summary>
		/// 尝试从队列头获取一个数据，成功后此数据也会从队列中出栈
		/// </summary>
		/// <param name="t">数据</param>
		/// <returns>是否成功拿到数据</returns>
		bool try_wait(T& t)
		{
			if (m_sem->TryWaitOne())
			{
				while (!m_queue.try_pop_front(t))
				{
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				}
				return true;
			}
			return false;
		}
	};
}