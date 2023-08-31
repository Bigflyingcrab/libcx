#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include "../common/cx_common_fun.h"
#include "../time/cx_time.h"
#include "../thread/cx_thread_blockqueue.h"

namespace cx
{
	class cxSqlData : public std::enable_shared_from_this<cxSqlData>
	{
	public:
		using Ptr = std::shared_ptr<cxSqlData>;
	public:
		std::string				m_str = "";
		bool					m_bnull = false;
	public:
		cxSqlData()
		{
			m_bnull = true;
		}
		cxSqlData(std::string str)
		{
			m_str = str;
		}
		~cxSqlData()
		{

		}
		bool isNull()
		{
			return m_bnull;
		}
		template<typename T> T toNum()
		{
			if (m_bnull)
			{
				return 0;
			}
			return cxCommonFun::StringToObject<T>(m_str);
		}
		bool toBool()
		{
			bool bret = false;
			if (m_str == "true" || m_str == "True" || m_str == "TRUE")
			{
				bret = true;
			}
			else if (m_str != "0" && m_str != "")
			{
				bret = true;
			}
			return bret;
		}
		cxDateTime toDateTime(std::string format = "%YYYY-%MM-%DD %hh:%mm:%ss")
		{
			if (m_bnull)
			{
				return cxDateTime();
			}
			cxDateTime dt = cxDateTime::FromString(m_str, format);
			return dt;
		}
		std::string toString()
		{
			if (m_bnull)
			{
				return "";
			}
			return m_str;
		}
	};

	class cxSqlHelper
	{
	public:
		using DataList = std::vector<std::map<std::string, cxSqlData::Ptr>>;
	protected:
		std::string												m_typeName = "";
		std::string												m_strConnect = "";
		std::string												m_file = "";
		std::string												m_ip = "";
		int														m_port = 0;
		std::string												m_user = "";
		std::string												m_pass = "";
		std::string												m_db_name = "";
	public:
		virtual ~cxSqlHelper()
		{

		}
		void InitParam(const std::string& file,
			const std::string& ip, int port,
			const std::string& user, const std::string& pass,
			const std::string& db_name)
		{
			m_file = file;
			m_ip = ip;
			m_port = port;
			m_user = user;
			m_pass = pass;
			m_db_name = db_name;
		}
		virtual bool Connect() = 0;
		virtual bool DisConnect() = 0;
		virtual bool IsConnected() = 0;
		virtual bool Begin() = 0;
		virtual bool Commit() = 0;
		virtual bool RollBack() = 0;
		virtual bool Updata(const std::string& str_sql, int& count) = 0;
		virtual bool Search(const std::string& str_sql, DataList& list) = 0;
	};

	inline bool Sql_DangerousString(std::string str)
	{
		std::string key[11] = { "*","=","%0a","%"," union","|","&","^" ,"#","/*","*/" };
		for (int i = 0; i < 11; i++)
		{
			if (str.find(key[i]) != std::string::npos)
			{
				return true;
			}
		}
		return false;
	}
}
