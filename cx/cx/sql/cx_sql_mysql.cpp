#include "cx_sql_mysql.h"
#include <mysql/mysql.h>
#include "../loger/cx_loger.h"

namespace cx
{
	cxSqlMysql::cxSqlMysql()
	{

	}
	cxSqlMysql::~cxSqlMysql()
	{
		DisConnect();
	}
	bool cxSqlMysql::Connect()
	{
		if (IsConnected())
		{
			return true;
		}

		m_sql = mysql_init(NULL);
		if (m_sql == NULL)
		{
			cxLogerError("连接数据库失败，数据库连接句柄初始化失败");
			return false;
		}

		if (mysql_real_connect((MYSQL*)m_sql, m_ip.c_str(), m_user.c_str(), m_pass.c_str(), m_db_name.c_str(), m_port, NULL, 0) != m_sql)
		{
			cxLogerError("连接数据库失败，连接失败");
			return false;
		}

		if (mysql_set_character_set((MYSQL*)m_sql, "utf8") != 0)
		{
			cxLogerWarning("设置数据库连接字符集失败");
		}

		return true;
	}
	bool cxSqlMysql::DisConnect()
	{
		if (m_sql != NULL)
		{
			mysql_close((MYSQL*)m_sql);
			m_sql = NULL;
		}
		return true;
	}
	bool cxSqlMysql::IsConnected()
	{
		if (m_sql != NULL)
		{
			const char* buf = mysql_stat((MYSQL*)m_sql);
			if (buf == NULL)
			{
				mysql_close((MYSQL*)m_sql);
				m_sql = NULL;
				return false;
			}
			return true;
		}
		return false;
	}
	bool cxSqlMysql::Begin()
	{
		int ret = 0;
		return Updata("BEGIN;", ret);
	}
	bool cxSqlMysql::Commit()
	{
		int ret = 0;
		return Updata("COMMIT;", ret);
	}
	bool cxSqlMysql::RollBack()
	{
		int ret = 0;
		return Updata("ROLLBACK;", ret);
	}
	bool cxSqlMysql::Updata(const std::string& str_sql, int& count)
	{
		if (mysql_query((MYSQL*)m_sql, str_sql.c_str()) != 0)
		{
			std::string error(mysql_error((MYSQL*)m_sql));
			cxLogerError("执行mysql更新语句失败，mysql: " << error << "，sqlstr: " << str_sql);
			return false;
		}

		MYSQL_RES* result = mysql_store_result((MYSQL*)m_sql);
		if (result != NULL)
		{
			mysql_free_result(result);
		}

		count = ((MYSQL*)m_sql)->affected_rows;

		return true;
	}
	bool cxSqlMysql::Search(const std::string& str_sql, DataList& list)
	{
		if (mysql_query((MYSQL*)m_sql, str_sql.c_str()) != 0)
		{
			std::string error(mysql_error((MYSQL*)m_sql));
			cxLogerError("执行mysql检索语句失败，mysql: " << error << "，sqlstr: " << str_sql);
			return false;
		}

		MYSQL_RES* result;
		MYSQL_ROW sql_row;
		std::string name = "";

		result = mysql_store_result((MYSQL*)m_sql);
		if (result == NULL)
		{
			return false;
		}

		while (result && (sql_row = mysql_fetch_row(result)))
		{
			std::map<std::string, cxSqlData::Ptr> mdata;
			for (unsigned int i = 0; i < result->field_count; i++)
			{
				cxSqlData::Ptr md = NULL;

				if (sql_row[i] == NULL)
				{
					md = std::make_shared<cxSqlData>();
				}
				else if (strlen(sql_row[i]) == 0 && sql_row[i][0] == 1)
				{
					md = std::make_shared<cxSqlData>("1");
				}
				else
				{
					md = std::make_shared<cxSqlData>(std::string(sql_row[i]));
				}
				mdata[result->fields[i].name] = md;
			}
			list.push_back(mdata);
		}

		mysql_free_result(result);

		return true;
	}
}