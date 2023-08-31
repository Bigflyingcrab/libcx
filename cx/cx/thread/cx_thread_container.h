#pragma once
#include <iostream>
#include <memory>
#include <queue>
#include <list>
#include <mutex>
#include <map>
#include <future>
#include <functional>

namespace cx
{
	template<typename Key, typename Value> class cxSafeMap
	{
	private:
		typedef typename std::map<Key, Value>	TMap;
		typedef typename TMap::iterator			iterator;
	private:
		TMap									m_map;
		std::mutex								m_mutex;
	public:
		cxSafeMap()
		{

		}
		cxSafeMap(const cxSafeMap& sm)
		{
			m_map = sm.m_map;
		}
		~cxSafeMap()
		{

		}
		//Value& operator[](const Key& key)
		//{
		//	std::unique_lock<std::mutex> Lock(m_mutex);
		//	return m_map[key];
		//}
		void set(const Key& key, const Value& value)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_map[key] = value;
		}
		bool find(const Key& key)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_map.find(key) != m_map.end();
		}
		Value get(const Key& key)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			Value value = m_map[key];
			return value;
		}
		Value get_value(unsigned int index)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			auto it = m_map.begin();
			std::advance(it, index);
			Value value = it->second;
			return value;
		}
		Key get_key(unsigned int index)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			auto it = m_map.begin();
			std::advance(it, index);
			Key key = it->first;
			return key;
		}
		unsigned int size()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_map.size();
		}
		bool empty()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_map.empty();
		}
		void clear()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_map.clear();
		}
	};
	/// <summary>
	/// 线程安全队列
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T> class cxSafeQueue
	{
	public:
		using Ptr = std::shared_ptr<cxSafeQueue<T>>;
	private:
		std::queue<T>				m_queue;
		std::mutex					m_mutex;
	public:
		cxSafeQueue()
		{

		}
		~cxSafeQueue()
		{

		}
		/// <summary>
		/// 向队列中添加一个数据
		/// </summary>
		/// <param name="t">数据</param>
		void push(const T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);			
			m_queue.push(t);
		}
		/// <summary>
		/// 尝试从队列头获取并出栈一个数据
		/// </summary>
		/// <param name="t">出栈的数据</param>
		/// <returns>是否成功</returns>
		bool try_pop_front(T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			if (m_queue.empty())
			{
				return false;
			}
			t = m_queue.front();
			m_queue.pop();
			return true;
		}
		/// <summary>
		/// 从队列头获取并出栈一个数据
		/// 如果队列是空的，会抛出异常
		/// </summary>
		/// <returns></returns>
		T pop_front()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			T t = m_queue.front();
			m_queue.pop();
			return t;
		}
		/// <summary>
		/// 获取队列头的数据
		/// 如果队列是空的，会抛出异常
		/// </summary>
		/// <returns>数据</returns>
		T& front()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_queue.front();
		}
		/// <summary>
		/// 从队列头出栈一个数据
		/// 如果队列是空的，会抛出异常
		/// </summary>
		void pop()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_queue.pop();
		}
		/// <summary>
		/// 判断队列是否是空的
		/// </summary>
		/// <returns>是否为空</returns>
		bool empty()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_queue.empty();
		}
		/// <summary>
		/// 获取队列的数据数量
		/// </summary>
		/// <returns></returns>
		unsigned int size()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_queue.size();
		}
	};

	/// <summary>
	/// 线程安全列表
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T> class cxSafeList
	{
	public:
		using Ptr = std::shared_ptr<cxSafeList<T>>;
	private:
		std::list<T>				m_list;
		std::mutex					m_mutex;
	public:
		cxSafeList()
		{

		}
		~cxSafeList()
		{

		}
		/// <summary>
		/// 将数据添加至列表末尾
		/// </summary>
		/// <param name="t"></param>
		void push_back(const T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.push_back(t);
		}
		/// <summary>
		/// 将数据添加至列表头
		/// </summary>
		/// <param name="t"></param>
		void push_front(const T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.push_front(t);
		}
		/// <summary>
		/// 获取列表头的数据
		/// 如果列表是空的，会抛出异常
		/// </summary>
		/// <returns></returns>
		T& front()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_list.front();
		}
		/// <summary>
		/// 获取列表末尾的数据
		/// 如果列表是空的，会抛出异常
		/// </summary>
		/// <returns>数据</returns>
		T& back()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_list.back();
		}
		/// <summary>
		/// 尝试获取列表头的数据
		/// </summary>
		/// <param name="t">数据</param>
		/// <returns>是否获取成功</returns>
		bool try_front(T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			if (m_list.empty())
			{
				return false;
			}
			t = m_list.front();
			return true;
		}
		/// <summary>
		/// 尝试获取列表末尾的数据
		/// </summary>
		/// <param name="t">数据</param>
		/// <returns>是否成功获取</returns>
		bool try_back(T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			if (m_list.empty())
			{
				return false;
			}
			t = m_list.back();
			return true;
		}
		/// <summary>
		/// 列表中数据的数量
		/// </summary>
		/// <returns></returns>
		unsigned int size()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_list.size();
		}
		/// <summary>
		/// 列表是否为空
		/// </summary>
		/// <returns></returns>
		bool empty()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			return m_list.empty();
		}
		/// <summary>
		/// 将列表头的数据出栈
		/// 如果列表是空的，会抛出异常
		/// </summary>
		void pop_front()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.pop_front();
		}
		/// <summary>
		/// 将列表末尾的数据出栈
		/// 如果列表是空的，会抛出异常
		/// </summary>
		void pop_back()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.pop_back();
		}
		/// <summary>
		/// 以指定数据的方式在列表中移除一个数据，所有符合条件的数据都会被移除
		/// </summary>
		/// <param name="t">要移除的数据</param>
		void remove(const T& t)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			auto it = m_list.begin();
			while (it != m_list.end())
			{
				if (*it == t)
				{
					m_list.remove(it++);
					continue;
				}
				it++;
			}
		}
		/// <summary>
		/// 以指定下标的方式在列表中移除一个数据
		/// 如果列表为空或者下标越界，会抛出异常
		/// </summary>
		/// <param name="index"></param>
		void remove(const unsigned int index)
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.remove(index);
		}
		/// <summary>
		/// 清空列表
		/// </summary>
		void clear()
		{
			std::unique_lock<std::mutex> Lock(m_mutex);
			m_list.clear();
		}
	};
}
