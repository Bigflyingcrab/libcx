#pragma once
#include "cx_sql_helper.h"

namespace cx
{
	class cxSqlConnect
	{
	public:
		using Ptr = std::shared_ptr<cxSqlConnect>;
	private:
		std::shared_ptr<cxSqlHelper>			m_ptr = NULL;
		std::string								m_strConnect = "";
	public:
		cxSqlConnect(const std::string& typeName, const std::string& strConnect, unsigned int time_out);
		~cxSqlConnect();
		bool IsNull();
		cxSqlHelper* operator->();
	};

	class cxSqlPool
	{
	public:
		using Ptr = std::shared_ptr<cxSqlPool>;
	private:
		std::mutex												m_mutex;
		int														m_count = 0;
		cxBlockQueue<std::shared_ptr<cxSqlHelper>>				m_free;
		std::string												m_typeName = "";
		std::string												m_strConnect = "";
		std::string												m_file = "";
		std::string												m_ip = "";
		int														m_port = 0;
		std::string												m_user = "";
		std::string												m_pass = "";
		std::string												m_db_name = "";
		unsigned int											m_min = 1;
		unsigned int											m_max = 5;
	public:
		cxSqlPool();
		~cxSqlPool();
		static std::shared_ptr<cxSqlHelper> GetConnect(const std::string& typeName, const std::string& strConnect, unsigned int time_out = 0);
		static void BackConnect(const std::string& strConnect, std::shared_ptr<cxSqlHelper> ptr);
	private:
		std::shared_ptr<cxSqlHelper> CreateConnect();
		void ParseStrConnect(const std::string& strConnect);
	private:
		static Ptr CreatePool(const std::string& typeName, const std::string& strConnect);
	};
}
