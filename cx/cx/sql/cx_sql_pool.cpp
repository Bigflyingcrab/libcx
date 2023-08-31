#include "cx_sql_pool.h"
#include "cx_sql_mysql.h"
#include "../loger/cx_loger.h"

namespace cx
{
	cxSqlConnect::cxSqlConnect(const std::string& typeName, const std::string& strConnect, unsigned int time_out)
	{
		m_strConnect = typeName + strConnect;
		m_ptr = cxSqlPool::GetConnect(typeName, strConnect, time_out);
	}
	cxSqlConnect::~cxSqlConnect()
	{
		cxSqlPool::BackConnect(m_strConnect, m_ptr);
	}
	bool cxSqlConnect::IsNull()
	{
		return m_ptr == NULL;
	}
	cxSqlHelper* cxSqlConnect::operator->()
	{
		return m_ptr.get();
	}

	cxSqlPool::cxSqlPool()
	{

	}
	cxSqlPool::~cxSqlPool()
	{

	}

	cxSafeMap<std::string, std::shared_ptr<cxSqlPool>>			g_sqlpool;
	std::mutex													g_sqlpool_mutex;

	std::shared_ptr<cxSqlHelper> cxSqlPool::GetConnect(const std::string& typeName, const std::string& strConnect, unsigned int time_out)
	{
		std::shared_ptr<cxSqlPool> pool = CreatePool(typeName, strConnect);
		if (pool == NULL)
		{
			cxLogerError("获取数据库连接失败，连接池创建失败");
			return NULL;
		}

		std::shared_ptr<cxSqlHelper> conn = NULL;

		while (true)
		{
			if (pool->m_free.empty())
			{
				conn = pool->CreateConnect();
				if (conn == NULL)
				{
					if (!pool->m_free.wait(conn, time_out))
					{
						cxLogerError("获取数据库连接失败，获取连接超时");
						return NULL;
					}
					break;
				}
			}
			else
			{
				if (!pool->m_free.try_wait(conn))
				{
					continue;
				}
				break;
			}
		}

		for (int i = 0; i < 10; i++)
		{
			if (!conn->Connect())
			{
				cxLogerError("获取数据库连接失败，连接数据库失败。");
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				continue;
			}
			break;
		}

		if (!conn->IsConnected())
		{
			cxLogerError("获取数据库连接失败，获取连接超时");
			return NULL;
		}

		return conn;
	}
	void cxSqlPool::BackConnect(const std::string& strConnect, std::shared_ptr<cxSqlHelper> ptr)
	{
		std::shared_ptr<cxSqlPool> pool = g_sqlpool.get(strConnect);
		if (pool == NULL)
		{
			cxLogerError("归还数据库连接失败，没有找到对应的连接池");
			return;
		}
		pool->m_free.push(ptr);
	}
	std::shared_ptr<cxSqlHelper> cxSqlPool::CreateConnect()
	{
		std::unique_lock<std::mutex> Lock(m_mutex);
		if (m_count >= m_max)
		{
			return NULL;
		}

		std::shared_ptr<cxSqlHelper> ret = NULL;

		if (m_typeName == "mysql")
		{
			ret = std::make_shared<cxSqlMysql>();
		}

		ret->InitParam(m_file, m_ip, m_port, m_user, m_pass, m_db_name);

		return ret;
	}
	void cxSqlPool::ParseStrConnect(const std::string& strConnect)
	{
		std::vector<std::string> group;
		cxCommonFun::StringToGroup(strConnect, ";", group);
		for (unsigned int i = 0; i < group.size(); i++)
		{
			std::vector<std::string> note;
			cxCommonFun::StringToGroup(group[i], "=", note);
			if (note.size() != 2)
			{
				continue;
			}

			if (note[0] == "type_name") m_typeName = note[1];
			else if (note[0] == "file_path") m_file = note[1];
			else if (note[0] == "ip") m_ip = note[1];
			else if (note[0] == "port") m_port = cxCommonFun::StringToObject<int>(note[1]);
			else if (note[0] == "user") m_user = note[1];
			else if (note[0] == "pass") m_pass = note[1];
			else if (note[0] == "db_name") m_db_name = note[1];
			else if (note[0] == "min") m_min = cxCommonFun::StringToObject<int>(note[1]);
			else if (note[0] == "max") m_max = cxCommonFun::StringToObject<int>(note[1]);
		}
	}
	cxSqlPool::Ptr cxSqlPool::CreatePool(const std::string& typeName, const std::string& strConnect)
	{
		std::shared_ptr<cxSqlPool> pool = NULL;

		std::unique_lock<std::mutex> Lock(g_sqlpool_mutex);
		if (!g_sqlpool.find(typeName + strConnect))
		{
			pool = std::make_shared<cxSqlPool>();
			pool->m_typeName = typeName;
			pool->m_strConnect = strConnect;
			pool->ParseStrConnect(strConnect);
			if (pool->m_max < pool->m_min || pool->m_min < 0)
			{
				cxLogerError("创建数据库连接池失败，连接池参数有误");
				return NULL;
			}

			for (int i = 0; i < pool->m_min; i++)
			{
				std::shared_ptr<cxSqlHelper> conn = pool->CreateConnect();
				if (conn == NULL)
				{
					continue;
				}
				pool->m_free.push(conn);
			}

			g_sqlpool.set(typeName + strConnect, pool);
		}
		else
		{
			pool = g_sqlpool.get(typeName + strConnect);
		}

		return pool;
	}
}