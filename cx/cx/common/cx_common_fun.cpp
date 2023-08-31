#include "cx_common_fun.h"
#include <sys/syscall.h>
#include "../loger/cx_loger.h"
#include <openssl/err.h>
#include <openssl/evp.h>  
#include <openssl/bio.h> 
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/des.h>
#include <openssl/x509v3.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include "cx_guid.h"
#include "cx_command_proxy.h"
#include <fstream>

#include <sys/stat.h>

namespace cx
{
	std::string						cxCommonFun::m_AppPath = "";
	std::mutex						cxCommonFun::m_AppPath_mutex;

	void cxCommonFun::StringToLineGroup(const std::string& str, std::vector<std::string>& lines)
	{
		std::string value = "";
		for (unsigned int i = 0; i < str.length(); i++)
		{
			if (str[i] == '\r' || str[i] == '\n')
			{
				if (value != "")
				{
					lines.push_back(value);
					value = "";
				}
				continue;
			}
			value += str[i];
		}
		if (value != "")
		{
			lines.push_back(value);
		}
	}

	std::string cxCommonFun::StringFormat(std::string str)
	{
		while (str != "")
		{
			if (str[str.length() - 1] == '\r' || str[str.length() - 1] == '\n')
			{
				str = str.substr(0, str.length() - 1);
			}
			else if (str[0] == ' ')
			{
				str = str.substr(1);
			}
			else
			{
				break;
			}
		}
		return str;
	}

	void cxCommonFun::StringToGroup(const std::string& str, const std::string& part, std::vector<std::string>& group)
	{
		if (part == "")
		{
			group.push_back(str);
			return;
		}
		std::string value = "";
		bool bpart = false;
		int index = 0;

		for (unsigned int i = 0; i < str.length(); i++)
		{
			index = str.find(part, i);
			if (index == i)
			{
				if (!bpart)
				{
					bpart = true;
					if (value != "")
					{
						group.push_back(value);
						value = "";
						i += part.size() - 1;
						continue;
					}
				}
				i += part.length() - 1;
			}
			else
			{
				value += str[i];
				bpart = false;
			}
		}

		if (value != "")
		{
			group.push_back(value);
		}
	}

	bool cxCommonFun::StringIsNum(const std::string& str)
	{
		for (unsigned int i = 0; i < str.length(); i++)
		{
			if (str[i] < '0' || str[i] > '9')
			{
				return false;
			}
		}
		return true;
	}

	unsigned int cxCommonFun::StringFindCount(const std::string& str, const std::string& key)
	{
		int ret = 0;
		int index = 0;
		while (true)
		{
			index = str.find(key, index);
			if (index >= 0)
			{
				ret++;
				index++;
			}
			else
			{
				return ret;
			}
		}
	}

	bool cxCommonFun::SystemCommand(std::string command, std::vector<std::string>& res, std::string sudo_pass)
	{
		if (command == "")
		{
			return false;
		}

		if (sudo_pass != "")
		{
			command = "echo " + sudo_pass + " | sudo -S " + command;
		}

		command = "echo " + CreateGuid() + " && " + command;

		FILE* fp = popen(command.c_str(), "r");
		char buf[16384];
		while (true)
		{
			memset(buf, 0, sizeof(buf));
			if (fgets(buf, sizeof(buf), fp) == NULL)
			{
				break;
			}
			std::string str(buf);
			while (str != "" && (str[str.length() - 1] == '\r' || str[str.length() - 1] == '\n'))
			{
				str = str.substr(0, str.length() - 1);
			}
			res.push_back(std::string(str));
		}

		if (res.size() >= 1)
		{
			res.erase(res.begin());
		}
		int iret = pclose(fp);
		return true;
	}

	bool cxCommonFun::SystemCommand(std::string command, std::string sudo_pass)
	{
		std::vector<std::string> res;
		return SystemCommand(command, res, sudo_pass);
	}

	long long cxCommonFun::GetTickTime()
	{
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (long long)ts.tv_sec * (long long)1000 + (long long)ts.tv_nsec / (long long)1000000;
	}

	long long cxCommonFun::GetTickMicroTime()
	{
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (long long)ts.tv_sec * (long long)1000000 + (long long)ts.tv_nsec / (long long)1000;
	}

	long long cxCommonFun::GetTickTimeReal()
	{
		timespec ts;
		memset(&ts, 0, sizeof(timespec));
		clock_gettime(CLOCK_REALTIME, &ts);
		return (long long)ts.tv_sec * (long long)1000 + (long long)ts.tv_nsec / (long long)1000000;
	}

	std::string cxCommonFun::GetAppPath()
	{
		if (m_AppPath != "") return m_AppPath;
		std::unique_lock<std::mutex> Lock(m_AppPath_mutex);
		if (m_AppPath != "") return m_AppPath;
		char current_absolute_path[8192];
		memset(current_absolute_path, 0, sizeof(current_absolute_path));
		readlink("/proc/self/exe", current_absolute_path, sizeof(current_absolute_path));
		std::string str(current_absolute_path);
		int index = str.find_last_of("/");
		if (index > 0)
		{
			str = str.substr(0, index);
		}
		if (str != "")
		{
			m_AppPath = str;
		}
		return m_AppPath;
	}

	std::string cxCommonFun::GetFullPath(const std::string& path)
	{
		if (path == "")
		{
			return "";
		}

		if (path[0] == '/')
		{
			return path;
		}

		return GetAppPath() + "/" + path;
	}

	int cxCommonFun::GetCpuCount()
	{
		return sysconf(_SC_NPROCESSORS_ONLN);
	}

	bool cxCommonFun::IsHaveDirectory(const std::string& path)
	{
		if (path == "") return false;
		if (access(path.c_str(), F_OK) != 0)
		{
			return false;
		}
		return true;
	}

	bool cxCommonFun::IsHaveFile(const std::string& file_path)
	{
		if (file_path == "") return false;
		if (access(file_path.c_str(), F_OK) != 0)
		{
			return false;
		}
		return true;
	}

	bool cxCommonFun::CreateDirectory(std::string path)
	{
		if (path == "") return false;
		path = GetFullPath(path);
		std::vector<std::string> list;
		while (true)
		{
			int index = path.find('/');
			if (index < 0)
			{
				if (path != "")
				{
					list.push_back(path);
				}
				break;
			}
			if (index == 0)
			{
				path = path.substr(1);
				continue;
			}
			if (index > 0)
			{
				std::string temp = path.substr(0, index);
				list.push_back(temp);
				path = path.substr(index + 1);
				continue;
			}
		}
		if (list.size() == 0) return false;
		std::string temp_path = "";
		for (unsigned int i = 0; i < list.size(); i++)
		{
			temp_path += "/" + list[i];
			int isCreate = mkdir(temp_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
			if (isCreate != 0)
			{
				if (access(temp_path.c_str(), F_OK) != 0)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool cxCommonFun::RemoveDirectory(std::string path)
	{
		if (path == "") return false;
		path = GetFullPath(path);
		return rmdir(path.c_str()) == 0;
	}

	bool cxCommonFun::RemoveFile(std::string file_path)
	{
		if (file_path == "") return false;
		file_path = GetFullPath(file_path);
		return remove(file_path.c_str()) == 0;
	}

	std::string cxCommonFun::ReadTxtFile(std::string file_path)
	{
		if (file_path == "")
		{
			return "";
		}

		std::ifstream file(file_path, std::ios::in);
		if (!file.is_open())
		{
			return "";
		}

		file.seekg(0, std::ios::end);
		long long size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (size <= 0)
		{
			file.close();
			return "";
		}
		
		char* buf = new char[size];
		file.read(buf, size);
		std::string ret(buf, buf + size);
		delete buf;
		file.close();

		return ret;
	}

	bool cxCommonFun::GetCpuStatus(cpu_occupy_t& status)
	{
		FILE* fd;
		int n;
		char buff[256];
		cpu_occupy_t* cpu_occupy = &status;
		fd = fopen("/proc/stat", "r");
		if (fd == NULL)
		{
			perror("fopen:");
			return false;
		}
		fgets(buff, sizeof(buff), fd);
		sscanf(buff, "%s %lld %lld %lld %lld %lld %lld %lld %lld %lld",
			cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system,
			&cpu_occupy->idle, &cpu_occupy->iowait, &cpu_occupy->irq, &cpu_occupy->softirq, &cpu_occupy->stealstolen, &cpu_occupy->guest);
		fclose(fd);
		return true;
	}
	void read_cpu_times(long* user, long* nice, long* system, long* idle, long* iowait, long* irq, long* softirq) {
		FILE* fp;
		fp = fopen("/proc/stat", "r");
		if (fp == NULL) {
			printf("Error: cannot open /proc/stat.\n");
			exit(1);
		}

		fscanf(fp, "cpu %ld %ld %ld %ld %ld %ld %ld", user, nice, system, idle, iowait, irq, softirq);

		fclose(fp);
	}
	float cxCommonFun::GetCpuUsage(long long tick)
	{
		long user1, nice1, system1, idle1, iowait1, irq1, softirq1;
		long user2, nice2, system2, idle2, iowait2, irq2, softirq2;
		long prev_total, total, prev_idle, idle;
		float cpu_usage;

		// 读取第一次CPU时间信息
		read_cpu_times(&user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1);
		prev_total = user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1;
		prev_idle = idle1;

		// 等待一秒钟
		std::this_thread::sleep_for(std::chrono::milliseconds(tick));

		// 读取第二次CPU时间信息
		read_cpu_times(&user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2);
		total = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2;
		idle = idle2;

		// 计算CPU使用率
		cpu_usage = 100.0 * (1.0 - (float)(idle - prev_idle) * 1.0 / (float)(total - prev_total));
		return cpu_usage;
	}
	//float cxCommonFun::GetCpuUsage(long long tick)
	//{
	//	cpu_occupy_t cpu_stat1;
	//	cpu_occupy_t cpu_stat2;
	//	if (!GetCpuStatus(cpu_stat1))
	//	{
	//		return -1;
	//	}
	//	std::this_thread::sleep_for(std::chrono::milliseconds(tick));
	//	if (!GetCpuStatus(cpu_stat2))
	//	{
	//		return -1;
	//	}

	//	unsigned long long od, nd;
	//	od = (cpu_stat1.user + cpu_stat1.nice + cpu_stat1.system + cpu_stat1.idle + cpu_stat1.softirq + cpu_stat1.iowait + cpu_stat1.irq + cpu_stat1.stealstolen + cpu_stat1.guest);
	//	nd = (cpu_stat2.user + cpu_stat2.nice + cpu_stat2.system + cpu_stat2.idle + cpu_stat2.softirq + cpu_stat2.iowait + cpu_stat2.irq + cpu_stat2.stealstolen + cpu_stat2.guest);
	//	float cpu_use;

	//	if ((nd - od) != 0)
	//	{
	//		unsigned long long tol = nd - od;
	//		unsigned long long tol_ide = cpu_stat2.idle - cpu_stat1.idle;
	//		cpu_use = 100.0 * ((float)tol - (float)tol_ide) / (float)tol_ide;
	//	}
	//	else
	//		cpu_use = 0;
	//	return cpu_use;
	//}

	float cxCommonFun::GetMemoryUsage()
	{
		typedef struct MEMPACKED
		{
			char name1[20];
			unsigned long MemTotal;
			char name2[20];
			unsigned long MemFree;
			char name3[20];
			unsigned long MemAvailable;
		}MEM_OCCUPY;

		FILE* fd;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		MEM_OCCUPY mem;
		MEM_OCCUPY* m = &mem;

		fd = fopen("/proc/meminfo", "r");
		fgets(buff, sizeof(buff), fd);
		sscanf(buff, "%s %lu ", m->name1, &m->MemTotal);
		fgets(buff, sizeof(buff), fd);
		sscanf(buff, "%s %lu ", m->name2, &m->MemFree);
		fgets(buff, sizeof(buff), fd);
		sscanf(buff, "%s %lu ", m->name3, &m->MemAvailable);
		fclose(fd);

		return (float)(((float)mem.MemTotal - (float)mem.MemAvailable) / (float)mem.MemTotal) * 100.0;
	}

	std::string cxCommonFun::BitToNetMask(int bit)
	{
		unsigned char pp[4];
		memset(pp, 0, sizeof(pp));
		unsigned char* ptemp = pp;
		for (int i = 0; i < 32; i++)
		{
			if (i != 0 && i % 8 == 0)
			{
				ptemp++;
				if (i >= bit)
				{
					break;
				}
			}

			*ptemp = *ptemp << 1;
			if (i < bit)
			{
				*ptemp |= 1;
			}
		}

		std::stringstream ss;
		ss << (int)pp[0] << "." << (int)pp[1] << "." << (int)pp[2] << "." << (int)pp[3];
		return ss.str();
	}

	int cxCommonFun::NetMaskToBit(std::string str_mask)
	{
		if (str_mask == "")
		{
			return -1;
		}

		std::vector<std::string> group;
		StringToGroup(str_mask, ".", group);
		if (group.size() != 4)
		{
			return -1;
		}

		int ret = 0;
		for (int i = 0; i < 4; i++)
		{
			int temp = StringToObject<int>(group[i]);
			if (temp < 0 || temp > 255)
			{
				return -1;
			}
			unsigned char bit = (unsigned char)temp;

			for (int j = 0; j < 8; j++)
			{
				if (bit >= 128)
				{
					ret++;
				}
				else
				{
					break;
				}

				bit = bit << 1;
			}
		}

		return ret;
	}

	void __AddDiskChildren(Json::Value children, cxCommonFun::DiskInfo& disk)
	{
		for (int i = 0; i < children.size(); i++)
		{
			Json::Value item = children[i];
			cxCommonFun::DiskInfo di;
			di.uuid = JSON_STR(item, "uuid", "");
			di.name = JSON_STR(item, "name", "");
			di.serial = JSON_STR(item, "serial", "");
			di.maj_min = JSON_STR(item, "maj:min", "");
			di.fstype = JSON_STR(item, "fstype", "");
			di.ro = JSON_BOOL(item, "ro", false);
			di.rm = JSON_BOOL(item, "rm", false);
			di.size = JSON_INT64(item, "size", 0);
			if (di.size == 0)
			{
				di.size = cxCommonFun::StringToObject<long long>(JSON_STR(item, "size", ""));
			}
			di.type = JSON_STR(item, "type", "");

			if (!item["mountpoints"].isNull())
			{
				if (item["mountpoints"].isArray())
				{
					if (item["mountpoints"].size() > 0 && item["mountpoints"][0].isString())
					{
						di.mountpoint = item["mountpoints"][0].asString();
					}
				}
			}
			else if (!item["mountpoint"].isNull())
			{
				di.mountpoint = JSON_STR(item, "mountpoint", "");
			}
			if (!item["children"].isNull())
			{
				Json::Value cr = item["children"];
				__AddDiskChildren(cr, di);
			}
			disk.children.push_back(di);
		}
	}

	bool cxCommonFun::GetAllDiskInfo(std::vector<DiskInfo>& disks)
	{
		// 执行lsblk命令获取磁盘分区信息
		std::vector<std::string> res;
		int iret = cxCommandProxy::Command("lsblk -b -O --j", res);
		if (iret <= 0)
		{
			return false;
		}

		std::string str_json = "";
		for (unsigned int i = 0; i < res.size(); i++)
		{
			str_json += res[i];
		}

		Json::Value json;
		Json::Reader reader;
		if (!reader.parse(str_json, json) || json.isNull() || json["blockdevices"].isNull() || !json["blockdevices"].isArray())
		{
			return false;
		}

		Json::Value blockdevices = json["blockdevices"];

		for (int i = 0; i < blockdevices.size(); i++)
		{
			Json::Value json_disk = blockdevices[i];
			if (json_disk.isNull())
			{
				continue;
			}

			DiskInfo di;
			di.uuid = JSON_STR(json_disk, "uuid", "");
			di.name = JSON_STR(json_disk, "name", "");
			di.serial = JSON_STR(json_disk, "serial", "");
			di.maj_min = JSON_STR(json_disk, "maj:min", "");
			di.fstype = JSON_STR(json_disk, "fstype", "");
			di.ro = JSON_BOOL(json_disk, "ro", false);
			di.rm = JSON_BOOL(json_disk, "rm", false);
			di.size = JSON_INT64(json_disk, "size", 0);
			if (di.size == 0)
			{
				di.size = StringToObject<long long>(JSON_STR(json_disk, "size", ""));
			}
			di.type = JSON_STR(json_disk, "type", "");

			if (!json_disk["mountpoints"].isNull())
			{
				if (json_disk["mountpoints"].isArray())
				{
					if (json_disk["mountpoints"].size() > 0 && json_disk["mountpoints"][0].isString())
					{
						di.mountpoint = json_disk["mountpoints"][0].asString();
					}
				}
			}
			else if (!json_disk["mountpoint"].isNull())
			{
				di.mountpoint = JSON_STR(json_disk, "mountpoint", "");
			}

			if (di.type != "disk" || di.mountpoint == "[SWAP]")
			{
				continue;
			}

			if (!json_disk["children"].isNull())
			{
				Json::Value children = json_disk["children"];
				if (!children.isNull())
				{
					__AddDiskChildren(children, di);
				}
			}
			disks.push_back(di);
		}

		return true;
	}
	bool cxCommonFun::GetDiskInfoWithName(DiskInfo& info, std::string dev_name)
	{
		std::vector<DiskInfo> disks;
		if (!GetAllDiskInfo(disks))
		{
			return false;
		}

		for (unsigned int i = 0; i < disks.size(); i++)
		{
			if (disks[i].name == dev_name)
			{
				info = disks[i];
				return true;
			}
		}

		return false;
	}
	bool cxCommonFun::GetDiskInfoWithSn(DiskInfo& info, std::string dev_sn)
	{
		std::vector<DiskInfo> disks;
		if (!GetAllDiskInfo(disks))
		{
			return false;
		}

		for (unsigned int i = 0; i < disks.size(); i++)
		{
			if (disks[i].serial == dev_sn)
			{
				info = disks[i];
				return true;
			}
		}

		return false;
	}
	bool cxCommonFun::GetSDDiskInfo(std::vector<DiskInfo>& disks)
	{
		std::vector<DiskInfo> infos;
		if (!GetAllDiskInfo(infos))
		{
			return false;
		}

		for (unsigned int i = 0; i < infos.size(); i++)
		{
			if (infos[i].name.size() == 3 && infos[i].name[0] == 's' && infos[i].name[1] == 'd')
			{
				disks.push_back(infos[i]);
			}
		}

		return true;
	}


	bool cxCommonFun::GetDiskUseInfo(std::string disk_name, long long& write_speed, long long& read_speed)
	{
		std::ifstream file("/sys/block/" + disk_name + "/stat");
		if (!file.is_open())
		{
			return false;
		}

		char buf[4096];
		memset(buf, 0, sizeof(buf));
		file.getline(buf, sizeof(buf));
		file.close();

		std::string strline(buf);

		std::vector<std::string> group;
		StringToGroup(strline, " ", group);
		if (group.size() <= 6)
		{
			return false;
		}

		long long read_sectors = StringToObject<long long>(group[2]);
		long long write_sectors = StringToObject<long long>(group[6]);

		read_speed = read_sectors * 512;
		write_speed = write_sectors * 512;

		return true;
	}
	bool cxCommonFun::GetDiskUseStatus(std::string disk_name, long long& write_speed, long long& read_speed)
	{
		long long start_read_sectors = 0;
		long long start_write_sectors = 0;
		long long end_read_sectors = 0;
		long long end_write_sectors = 0;

		if (!GetDiskUseInfo(disk_name, start_write_sectors, start_read_sectors))
		{
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (!GetDiskUseInfo(disk_name, end_write_sectors, end_read_sectors))
		{
			return false;
		}

		read_speed = (end_read_sectors - start_read_sectors) * 5;
		write_speed = (end_write_sectors - start_write_sectors) * 5;

		return true;
	}
	bool cxCommonFun::GetDiskUseStatus(long long& write_byte, long long& read_byte)
	{
		std::vector<cx::cxCommonFun::DiskInfo> find_disks;
		if (!cx::cxCommonFun::GetSDDiskInfo(find_disks) || find_disks.size() <= 0)
		{
			return false;
		}

		write_byte = 0;
		read_byte = 0;

		for (int i = 0; i < find_disks.size(); i++)
		{
			std::ifstream file("/sys/block/" + find_disks[i].name + "/stat");
			if (!file.is_open())
			{
				return false;
			}

			char buf[4096];
			memset(buf, 0, sizeof(buf));
			file.getline(buf, sizeof(buf));
			file.close();

			std::string strline(buf);

			std::vector<std::string> group;
			StringToGroup(strline, " ", group);
			if (group.size() <= 6)
			{
				continue;
			}

			long long read_sectors = StringToObject<long long>(group[2]);
			long long write_sectors = StringToObject<long long>(group[6]);

			write_byte += read_sectors * 512;
			read_byte += write_sectors * 512;
		}

		return true;
	}

	bool cxCommonFun::MountDisk_SN(std::string disk_sn, std::string path)
	{
		DiskInfo info;
		if (!GetDiskInfoWithSn(info, disk_sn))
		{
			return false;
		}

		if (info.mountpoint == path)
		{
			return true;
		}

		return MountDisk_Name(info.name, path);
	}
	bool cxCommonFun::MountDisk_Name(std::string device_name, std::string path)
	{
		std::vector<std::string> lines;
		std::stringstream command;

		if (!IsHaveDirectory(path))
		{
			if (!CreateDirectory(path))
			{
				return false;
			}
		}

		command.str("");
		lines.clear();
		command << "mount /dev/" << device_name << " " << path << " && chmod 777 " << path;
		int iret = cxCommandProxy::Command(command.str(), lines);
		if (iret <= 0)
		{
			return false;
		}

		if (lines.size() == 0)
		{
			return true;
		}

		bool bret = false;

		for (int i = 0; i < 10; i++)
		{
			command.str("");
			lines.clear();
			command << "df " << path;
			int iret = cxCommandProxy::Command(command.str(), lines);
			if (iret <= 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				continue;
			}

			if (lines.size() < 2)
			{
				bret = false;
				break;
			}

			if (lines[1].find("/dev/" + device_name) == 0)
			{
				bret = true;
				break;
			}
		}

		if (bret)
		{
			command.str("");
			lines.clear();
			command << "chmod 777 " << path;
			int iret = cxCommandProxy::Command(command.str(), lines);
			if (iret <= 0)
			{
				bret = false;
			}
		}
		
		return bret;
	}

	bool cxCommonFun::UMountDisk_SN(std::string disk_sn)
	{
		DiskInfo info;
		if (!GetDiskInfoWithSn(info, disk_sn))
		{
			return false;
		}

		if (info.mountpoint == "")
		{
			return true;
		}

		return UMountDisk_Name(info.name);
	}
	bool cxCommonFun::UMountDisk_Name(std::string device_name)
	{
		for (int i = 0; i < 10; i++)
		{
			DiskInfo info;
			if (!GetDiskInfoWithName(info, device_name))
			{
				continue;
			}

			if (info.mountpoint == "")
			{
				bool bflag = false;
				for (unsigned int i = 0; i < info.children.size(); i++)
				{
					if (info.children[i].mountpoint != "")
					{
						bflag = true;
						break;
					}
				}
				if (!bflag)
				{
					return true;
				}
			}

			std::vector<std::string> lines;
			std::stringstream command;
			command << "umount /dev/" << device_name << "*";
			int iret = cxCommandProxy::Command(command.str(), lines);
			if (iret <= 0)
			{
				continue;
			}

			if (lines.size() == 0)
			{
				return true;
			}
		}

		return false;
	}

	bool cxCommonFun::GetPathSizeInfo(std::string path, long long& size, long long& used, long long& free)
	{
		if (path == "")
		{
			return false;
		}

		if (path == ".")
		{
			path = GetAppPath();
		}

		std::vector<std::string> lines;
		std::stringstream command;
		command << "df -B1 " << path;
		int iret = cxCommandProxy::Command(command.str(), lines);
		if (iret <= 0 || lines.size() <= 1)
		{
			return false;
		}

		std::vector<std::string> groups;
		StringToGroup(lines[1], " ", groups);
		if (groups.size() < 6)
		{
			return false;
		}

		size = StringToObject<long long>(groups[1]);
		used = StringToObject<long long>(groups[2]);
		free = StringToObject<long long>(groups[3]);

		return true;
	}

	bool cxCommonFun::GetFileInfo(std::string path, FileInfo& info)
	{
		try
		{
			std::string full_path = path;
			struct stat s;
			int ret = lstat(path.c_str(), &s);
			if (ret != 0)
			{
				return false;
			}
			int index = path.find_last_of("/");
			if (index >= 0)
			{
				path = path.substr(index + 1);
			}
			index = path.find_last_of(".");
			if (index > 0)
			{
				info.name = path.substr(0, index);
				info.ext_name = path.substr(index + 1);
			}
			else
			{
				info.name = path;
			}
			info.filesize = s.st_size;
			info.createtime = cxDateTime::FromTimespec(s.st_ctim);
			info.lastaccesstime = cxDateTime::FromTimespec(s.st_atim);
			info.lastwritetime = cxDateTime::FromTimespec(s.st_mtim);
			std::ifstream file(full_path);
			if (file.is_open())
			{
				file.seekg(0, file.end);
				info.filesize = file.tellg();
				file.close();
			}
			return true;
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
			return false;
		}
	}

	long long cxCommonFun::GetFileSize(const std::string& path)
	{
		long long ret = -1;
		std::ifstream file(path);
		if (file.is_open())
		{
			file.seekg(0, file.end);
			ret = file.tellg();
			file.close();
		}

		return ret;
	}

	void cxCommonFun::GetFiles(std::string path, std::vector<FileInfo>& infos)
	{
		try
		{
			DIR* dir;
			struct dirent* dp;
			dir = opendir(path.c_str());
			if (dir == NULL)
			{
				return;
			}
			while (true)
			{
				dp = readdir(dir);
				if (dp == NULL)
				{
					break;
				}
				if (dp->d_type == DT_REG)
				{
					struct stat s;
					int ret = lstat((path + dp->d_name).c_str(), &s);
					if (ret == 0)
					{
						FileInfo fi;
						std::string file_name(dp->d_name);
						int index = file_name.find_last_of(".");
						if (index > 0)
						{
							fi.name = file_name.substr(0, index);
							fi.ext_name = file_name.substr(index + 1);
						}
						else
						{
							fi.name = path;
						}
						fi.filesize = s.st_size;
						fi.createtime = cxDateTime::FromTimespec(s.st_ctim);
						fi.lastaccesstime = cxDateTime::FromTimespec(s.st_atim);
						fi.lastwritetime = cxDateTime::FromTimespec(s.st_mtim);
						infos.push_back(fi);
					}
				}
			}
			closedir(dir);
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
		}
	}

	void cxCommonFun::GetDirectorys(std::string path, std::vector<std::string>& dirs)
	{
		try
		{
			DIR* dir;
			struct dirent* dp;
			dir = opendir(path.c_str());
			if (dir == NULL)
			{
				return;
			}
			while (true)
			{
				dp = readdir(dir);
				if (dp == NULL)
				{
					break;
				}
				if (dp->d_type == DT_DIR)
				{
					std::string dir_name(dp->d_name);
					if (dir_name != "." && dir_name != "..")
					{
						dirs.push_back(dp->d_name);
					}
				}
			}
			closedir(dir);
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
		}
	}

	bool g_init_rand = false;
	std::mutex g_init_rand_mutex;
	void __InitRand()
	{
		try
		{
			if (g_init_rand) return;
			std::unique_lock<std::mutex> Lock(g_init_rand_mutex);
			if (g_init_rand) return;
			g_init_rand = true;
			std::srand(time(NULL));
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
		}
	}

	int cxCommonFun::CreateRandNum(int min, int max)
	{
		try
		{
			if (min > max)
			{
				return 0;
			}
			if (min == max)
			{
				return min;
			}
			__InitRand();
			int num = std::rand();
			num = num % (max + 1 - min);
			num += min;
			return num;
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
			return 0;
		}
	}

	std::string cxCommonFun::CreateRandString(unsigned int length, std::string seed)
	{
		try
		{
			if (length == 0)
			{
				return "";
			}

			int min, max;
			if (seed == "")
			{
				seed = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`-=[]\\;',./~!@#$%^&*()_+{}|:\"<>?";
			}
			min = 0;
			max = seed.size() - 1;

			std::string ret = "";
			for (int i = 0; i < length; i++)
			{
				ret += seed[CreateRandNum(min, max)];
			}

			return ret;
		}
		catch (std::exception& e)
		{
			cxLogerException(e.what());
			return "";
		}
	}

	std::vector<char> cxCommonFun::Sha1(std::vector<char>& buf)
	{
		unsigned char temp[20] = { 0 };
		unsigned char* pret = SHA1((unsigned char*)buf.data(), buf.size(), temp);
		std::vector<char> ret(pret, pret + 20);
		return ret;
	}

	std::string cxCommonFun::Base64_Encodec(std::vector<char>& src)
	{
		return Base64_Encodec(src.data(), src.size());
	}
	std::string cxCommonFun::Base64_Encodec(char* p, unsigned int size)
	{
		if (p == NULL || size <= 0)
		{
			return "";
		}

		BIO* bmem = NULL;
		BIO* b64 = NULL;
		BUF_MEM* bptr;

		b64 = BIO_new(BIO_f_base64());
		bmem = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, bmem);
		BIO_write(b64, p, size);
		BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bptr);
		BIO_set_close(b64, BIO_NOCLOSE);

		char* buff = (char*)malloc(bptr->length + 1);
		memcpy(buff, bptr->data, bptr->length);
		buff[bptr->length] = 0;
		BIO_free_all(b64);

		std::string ret = std::string(buff);
		if (ret != "" && ret[ret.length() - 1] == '\n')
		{
			ret.erase(ret.end() - 1);
		}

		return ret;
	}

	std::string cxCommonFun::CreateToken()
	{
		cxGuid guid;
		return guid.toToken();
	}

	std::string cxCommonFun::CreateGuid()
	{
		cxGuid guid;
		return guid.toString();
	}

	unsigned long long cxCommonFun::GetProcessId()
	{
		return getpid();
	}

	bool cxCommonFun::GetNetDeviceInfo(std::vector<NetDeviceInfo>& infos, std::string device_name)
	{
		std::stringstream command;
		std::vector<std::string> lines;
		std::vector<NetDeviceInfo> temp;

		command << "nmcli device";
		int iret = cxCommandProxy::Command(command.str(), lines);
		if (iret <= 0 || lines.size() <= 1)
		{
			return false;
		}

		for (unsigned int i = 1; i < lines.size(); i++)
		{
			std::vector<std::string> group;
			StringToGroup(lines[i], " ", group);
			if (group.size() < 4)
			{
				continue;
			}

			// 非互联网的不要
			if (group[1] != "ethernet")
			{
				continue;
			}

			// 非托管的不要
			if (group[2] == "unmanaged")
			{
				continue;
			}

			// 非指定的设备不要
			if (device_name != "" && group[0] != device_name)
			{
				continue;
			}

			NetDeviceInfo nd;
			nd.name = group[0];
			for (unsigned int j = 3; j < group.size(); j++)
			{
				nd.connect_name += group[j];
				if (j < group.size() - 1)
				{
					nd.connect_name += '\\';
					nd.connect_name += ' ';
				}
			}

			if (group[2] == "connected" || group[2] == "已连接")
			{
				nd.connected = true;
				temp.push_back(nd);
			}
		}

		if (temp.size() <= 0)
		{
			return false;
		}

		for (unsigned int i = 0; i < temp.size(); i++)
		{
			command.str("");
			lines.clear();
			command << "nmcli connection show " << temp[i].connect_name << " | grep 'ipv4.method\\|IP4'";
			iret = cxCommandProxy::Command(command.str(), lines);

			for (unsigned int j = 0; j < lines.size(); j++)
			{
				std::vector<std::string> group;
				StringToGroup(lines[j], " ", group);
				if (group.size() < 2)
				{
					continue;
				}
				if (group[0] == "ipv4.method:")
				{
					temp[i].ip_mode = group[1];
				}
				else if (group[0] == "IP4.ADDRESS[1]:")
				{
					int index = group[1].find("/");
					if (index > 0)
					{
						temp[i].ip = group[1].substr(0, index);
						temp[i].netmask = BitToNetMask(StringToObject<int>(group[1].substr(index + 1)));
					}
				}
				else if (group[0] == "IP4.GATEWAY:")
				{
					temp[i].gateway = group[1];
				}
				else if (group[0] == "IP4.DNS[1]:")
				{
					temp[i].dns = group[1];
				}
			}

			command.str("");
			lines.clear();
			command << "ifconfig " << temp[i].name << " | grep broadcast";
			iret = cxCommandProxy::Command(command.str(), lines);
			if (iret > 0 && lines.size() > 0)
			{
				std::vector<std::string> group;
				StringToGroup(lines[0], " ", group);
				if (group.size() > 0)
				{
					temp[i].broadcast = group[group.size() - 1];
				}
			}

			infos.push_back(temp[i]);
		}

		return infos.size() > 0;
	}
	bool cxCommonFun::GetNetDeviceUseInfo(NetDeviceUseInfo& info, std::string device_name)
	{
		int speed = 0;
		long long send_bytes = 0;
		long long recv_bytes = 0;
		char buf[1024];
		std::ifstream file("/sys/class/net/" + device_name + "/speed");
		if (!file.is_open())
		{
			return false;
		}
		memset(buf, 0, sizeof(buf));
		if (!file.getline(buf, sizeof(buf)))
		{
			file.close();
			return false;
		}
		speed = StringToObject<int>(buf);
		file.close();

		file = std::ifstream("/sys/class/net/" + device_name + "/statistics/rx_bytes");
		if (!file.is_open())
		{
			return false;
		}
		memset(buf, 0, sizeof(buf));
		if (!file.getline(buf, sizeof(buf)))
		{
			file.close();
			return false;
		}
		recv_bytes = StringToObject<long long>(buf);
		file.close();

		file = std::ifstream("/sys/class/net/" + device_name + "/statistics/tx_bytes");
		if (!file.is_open())
		{
			return false;
		}
		memset(buf, 0, sizeof(buf));
		if (!file.getline(buf, sizeof(buf)))
		{
			file.close();
			return false;
		}
		send_bytes = StringToObject<long long>(buf);
		file.close();

		info.name = device_name;
		info.recv_bytes = recv_bytes;
		info.send_bytes = send_bytes;
		info.speed = speed;

		return true;
	}

	bool cxCommonFun::GetNetUseStatus(float& use_status, long long& ret_speed, long long& send_speed, long long& recv_speed)
	{
		std::vector<NetDeviceInfo> net;
		if (!GetNetDeviceInfo(net) || net.size() <= 0)
		{
			return false;
		}

		for (unsigned int i = 0; i < net.size(); i++)
		{
			if (!net[i].connected)
			{
				net.erase(net.begin() + i);
				i++;
			}
		}

		if (net.size() <= 0)
		{
			return false;
		}

		long long wait_time = 500;
		float speed = 0;
		long long start_send_bytes = 0;
		long long start_recv_bytes = 0;
		long long end_send_bytes = 0;
		long long end_recv_bytes = 0;
		long long send_bytes = 0;
		long long recv_bytes = 0;

		for (unsigned int i = 0; i < net.size(); i++)
		{
			NetDeviceUseInfo use_info;
			if (!GetNetDeviceUseInfo(use_info, net[i].name))
			{
				continue;
			}

			speed += use_info.speed;
			start_send_bytes += use_info.send_bytes;
			start_recv_bytes += use_info.recv_bytes;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));

		for (unsigned int i = 0; i < net.size(); i++)
		{
			NetDeviceUseInfo use_info;
			if (!GetNetDeviceUseInfo(use_info, net[i].name))
			{
				continue;
			}

			end_send_bytes += use_info.send_bytes;
			end_recv_bytes += use_info.recv_bytes;
		}

		speed = speed * 1000000.0 / 8.0;
		send_bytes = (double)(end_send_bytes - start_send_bytes) * (1000.0 / (double)wait_time);
		recv_bytes = (double)(end_recv_bytes - start_recv_bytes) * (1000.0 / (double)wait_time);

		ret_speed = speed;
		use_status = MAX<float>((float)send_bytes / speed, (float)recv_bytes / speed) * 100.0;
		send_speed = send_bytes;
		recv_speed = recv_bytes;
		return true;
	}

	bool cxCommonFun::GetNetUseStatus(long long& speed, long long& send_byte, long long& recv_byte)
	{
		std::vector<NetDeviceInfo> net;
		if (!GetNetDeviceInfo(net) || net.size() <= 0)
		{
			return false;
		}

		for (unsigned int i = 0; i < net.size(); i++)
		{
			if (!net[i].connected)
			{
				net.erase(net.begin() + i);
				i++;
			}
		}

		if (net.size() <= 0)
		{
			return false;
		}

		speed = 0;
		send_byte = 0;
		recv_byte = 0;

		for (unsigned int i = 0; i < net.size(); i++)
		{
			NetDeviceUseInfo use_info;
			if (!GetNetDeviceUseInfo(use_info, net[i].name))
			{
				continue;
			}

			speed += use_info.speed;
			send_byte += use_info.send_bytes;
			recv_byte += use_info.recv_bytes;
		}

		return true;
	}

	bool cxCommonFun::GetNetDeviceUseStatus(std::string device_name, unsigned int time, int& speed, long long& send_bytes, long long& recv_bytes)
	{
		NetDeviceUseInfo start;
		NetDeviceUseInfo end;

		if (!GetNetDeviceUseInfo(start, device_name))
		{
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(time));

		if (!GetNetDeviceUseInfo(end, device_name))
		{
			return false;
		}

		speed = start.speed;
		send_bytes = end.send_bytes - start.send_bytes;
		recv_bytes = end.recv_bytes - start.recv_bytes;

		return true;
	}

	float cxCommonFun::GetJetsonGpuUseStatus()
	{
		std::vector<std::string> res;
		if (!cxCommandProxy::Command("cat /sys/devices/gpu.0/load", res) || res.size() <= 0)
		{
			return 0;
		}

		int value = StringToObject<int>(res[0]);

		return (float)value / 10.0;
	}

	int cxCommonFun::GetJetsonFanSpeed()
	{
		std::vector<std::string> res;
		for (int i = 0; i < 10; i++)
		{
			if (!cxCommandProxy::Command("cat /sys/class/hwmon/hwmon*/rpm", res) || res.size() <= 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			break;
		}
		
		if (res.size() <= 0)
		{
			return 0;
		}

		return StringToObject<int>(res[0]);
	}

	bool cxCommonFun::GetJetsonTemperature(JetsonTemperature& info)
	{
		std::vector<std::string> res1, res2;

		for (int i = 0; i < 10; i++)
		{
			if (!cxCommandProxy::Command("cat /sys/devices/virtual/thermal/thermal_zone*/type", res1) || res1.size() <= 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			break;
		}

		if (res1.size() <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < 10; i++)
		{
			if (!cxCommandProxy::Command("cat /sys/devices/virtual/thermal/thermal_zone*/temp", res2) || res2.size() <= 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			break;
		}

		if (res2.size() <= 0)
		{
			return false;
		}		

		if (res1.size() != res2.size())
		{
			return false;
		}

		for (unsigned int i = 0; i < res1.size(); i++)
		{
			if (res1[i] == "CPU-therm") // cpu 温度
			{
				info.cpu_therm = cx::cxCommonFun::StringToObject<int>(res2[i]);
			}
			else if (res1[i] == "GPU-therm") // gpu 温度
			{
				info.gpu_therm = cx::cxCommonFun::StringToObject<int>(res2[i]);
			}
			else if (res1[i] == "AO-therm") // 主板温度
			{
				info.ao_therm = cx::cxCommonFun::StringToObject<int>(res2[i]);
			}
			else if (res1[i] == "PMIC-Die") // 供电模块温度
			{
				info.pmic_die = cx::cxCommonFun::StringToObject<int>(res2[i]);
			}
			else if (res1[i] == "thermal_fan_est") // 主散热片温度
			{
				info.thermal_fan_est = cx::cxCommonFun::StringToObject<int>(res2[i]);
			}
		}

		return true;
	}

	std::string cxCommonFun::Md5(std::string str)
	{
		unsigned char MD5result[16] = { 0 };
		MD5((const unsigned char*)str.data(), str.size(), MD5result);
		char temp[33] = { 0 };
		std::string md5 = "";
		for (int i = 0; i < 16; ++i)
		{
			snprintf(temp, 10, "%02x", MD5result[i]);
			md5 += temp;
		}
		return md5;
	}
}
