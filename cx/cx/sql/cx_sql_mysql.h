#pragma once
#include "cx_sql_helper.h"

namespace cx
{
	class cxSqlMysql : public cxSqlHelper
	{
	private:
		void*					m_sql = NULL;
	public:
		using Ptr = std::shared_ptr<cxSqlMysql>;
	public:
		cxSqlMysql();
		~cxSqlMysql();
		bool Connect();
		bool DisConnect();
		bool IsConnected();
		bool Begin();
		bool Commit();
		bool RollBack();
		bool Updata(const std::string& str_sql, int& count);
		bool Search(const std::string& str_sql, DataList& list);
	};
}