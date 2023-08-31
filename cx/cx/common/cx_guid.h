#pragma once
#include <iostream>
#include <memory>
#include <memory.h>
#include <exception>
#include <uuid/uuid.h>

namespace cx
{
	typedef unsigned char uuid_t[16];

	class cxGuid
	{
	public:
		using Ptr = std::shared_ptr<cxGuid>;
	private:
		uuid_t				m_id;
	public:
		cxGuid()
		{
			uuid_generate(m_id);
		}
		cxGuid(const cxGuid& id)
		{
			memcpy(m_id, id.m_id, 16);
		}
		~cxGuid()
		{

		}
		cxGuid& operator=(const cxGuid& id)
		{
			memcpy(m_id, id.m_id, 16);
			return *this;
		}
		cxGuid& operator=(const uuid_t id)
		{
			memcpy(m_id, id, 16);
			return *this;
		}
		bool operator==(const cxGuid& id)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] != id.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		friend bool operator==(const cxGuid& id1, const cxGuid& id2)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (id1.m_id[i] != id2.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		bool operator>(const cxGuid& id)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] <= id.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		friend bool operator>(const cxGuid& id1, const cxGuid& id2)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (id1.m_id[i] <= id2.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		bool operator>=(const cxGuid& id)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] < id.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		friend bool operator>=(const cxGuid& id1, const cxGuid& id2)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (id1.m_id[i] < id2.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		bool operator<(const cxGuid& id)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] >= id.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		friend bool operator<(const cxGuid& id1, const cxGuid& id2)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (id1.m_id[i] >= id2.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		bool operator<=(const cxGuid& id)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] > id.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		friend bool operator<=(const cxGuid& id1, const cxGuid& id2)
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (id1.m_id[i] > id2.m_id[i])
				{
					return false;
				}
			}
			return true;
		}
		bool IsNull()
		{
			for (unsigned int i = 0; i < 16; i++)
			{
				if (m_id[i] != 0)
				{
					return false;
				}
			}
			return true;
		}
		/// <summary>
		/// guid转为字符串，示例：ff88f6e8-f161-42b5-aa40-4a858c9e6ec3
		/// </summary>
		/// <returns></returns>
		std::string toString()
		{
			char str[36];
			uuid_unparse(m_id, str);
			return std::string(str);
		}
		/// <summary>
		/// guid转为没有'-'的字符串，示例：ff88f6e8f16142b5aa404a858c9e6ec3
		/// </summary>
		/// <returns></returns>
		std::string toToken()
		{
			std::string str = toString();
			while (true)
			{
				int index = str.find("-");
				if (index < 0)
				{
					break;
				}
				str.erase(index, 1);
			}
			return str;
		}
		/// <summary>
		/// 从字符串反向解析为guid类型，仅支持带'-'的类型，示例：ff88f6e8-f161-42b5-aa40-4a858c9e6ec3
		/// </summary>
		/// <param name="str"></param>
		/// <returns></returns>
		static inline cxGuid& Parse(std::string str)
		{
			cxGuid guid;
			uuid_parse(str.c_str(), guid.m_id);
			return guid;
		}
	};
}

