#pragma once
#include <iostream>
#include <stdio.h>
#include <memory>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include "../json/json.h"
#include "../time/cx_time.h"

#define							MAX_INT32							0x7FFFFFFF
#define							MIN_INT32							0xFFFFFFFF
#define							MAX_UINT32							0xFFFFFFFF
#define							MIN_UINT32							0
#define							MAX_SHORT							0x7FFF
#define							MIN_SHORT							0xFFFF
#define							MAX_USHORT							0xFFFF
#define							MIN_USHORT							0
#define							MAX_CHAR							0x7F
#define							MIN_CHAR							0xFF
#define							MAX_UCHAR							0xFF
#define							MIN_UCHAR							0

#define		JSON_STR(json_ent, key, default_value)			\
			(((json_ent)[(key)].isNull() || !(json_ent)[(key)].isString()) ? (default_value) : (json_ent)[(key)].asString())
#define		JSON_INT(json_ent, key, default_value)			\
			(((json_ent)[(key)].isNull() || !(json_ent)[(key)].isInt()) ? (default_value) : (json_ent)[(key)].asInt())
#define		JSON_INT64(json_ent, key, default_value)		\
			(((json_ent)[(key)].isNull() || !(json_ent)[(key)].isInt64()) ? (default_value) : (json_ent)[(key)].asInt64())
#define		JSON_BOOL(json_ent, key, default_value)			\
			(((json_ent)[(key)].isNull() || !((json_ent)[(key)].isBool())) ? (default_value) : (json_ent)[(key)].asBool())
#define		JSON_DOUBLE(json_ent, key, default_value)		\
			(((json_ent)[(key)].isNull() || !((json_ent)[(key)].isDouble())) ? (default_value) : (json_ent)[(key)].asDouble())

namespace cx
{
	class cxCommonFun
	{
	private:
		static std::string						m_AppPath;
		static std::mutex						m_AppPath_mutex;
	public:
		/// <summary>
		/// 获取最大值
		/// </summary>
		/// <typeparam name="T">自定义类型</typeparam>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		template<typename T> static inline T MAX(const T& a, const T& b)
		{
			return (a > b ? a : b);
		}

		/// <summary>
		/// 获取最小值
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		template<typename T> static inline T MIN(const T& a, const T& b)
		{
			return (a < b ? a : b);
		}

		/// <summary>
		/// 获取限定值
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="src"></param>
		/// <param name="lower"></param>
		/// <param name="maxer"></param>
		/// <returns></returns>
		template<typename T> static inline T CLAMP(const T& src, const T& lower, const T& maxer)
		{
			return src < lower ? lower : src > maxer ? maxer : src;
		}

		/// <summary>
		/// 字符串转其他类型
		/// </summary>
		/// <typeparam name="Object"></typeparam>
		/// <param name="str"></param>
		/// <returns></returns>
		template<typename Object> static inline Object StringToObject(const std::string& str)
		{
			Object ret;
			std::stringstream ss;
			ss << str;
			ss >> ret;
			return ret;
		}

		/// <summary>
		/// 其他类型转字符串
		/// </summary>
		/// <typeparam name="Object"></typeparam>
		/// <param name="obj"></param>
		/// <returns></returns>
		template<typename Object> static inline std::string ObjectToString(const Object& obj)
		{
			std::stringstream ss;
			ss << obj;
			return ss.str();
		}

		/// <summary>
		/// 字符串按行分组
		/// </summary>
		/// <param name="str"></param>
		/// <param name="lines"></param>
		static void StringToLineGroup(const std::string& str, std::vector<std::string>& lines);

		/// <summary>
		/// 格式化字符串
		/// </summary>
		/// <param name="str"></param>
		/// <returns></returns>
		static std::string StringFormat(std::string str);

		/// <summary>
		/// 字符串分组
		/// </summary>
		/// <param name="str"></param>
		/// <param name="part"></param>
		/// <param name="group"></param>
		static void StringToGroup(const std::string& str, const std::string& part, std::vector<std::string>& group);

		/// <summary>
		/// 判断字符串是否为数字字符串
		/// </summary>
		/// <param name="str"></param>
		/// <returns></returns>
		static bool StringIsNum(const std::string& str);

		/// <summary>
		/// 字符串获取字串数量
		/// </summary>
		/// <param name="str"></param>
		/// <param name="key"></param>
		/// <returns></returns>
		static unsigned int StringFindCount(const std::string& str, const std::string& key);
		
		/// <summary>
		/// 运行系统命令行命令
		/// </summary>
		/// <param name="command"></param>
		/// <param name="res"></param>
		/// <param name="sudo_pass"></param>
		/// <returns></returns>
		static bool SystemCommand(std::string command, std::vector<std::string>& res, std::string sudo_pass = "");

		/// <summary>
		/// 运行系统命令行命令
		/// </summary>
		/// <param name="command"></param>
		/// <param name="sudo_pass"></param>
		/// <returns></returns>
		static bool SystemCommand(std::string command, std::string sudo_pass = "");

		/// <summary>
		/// 获取相对时间戳，自系统启动以来到现在经过的毫秒
		/// </summary>
		/// <returns></returns>
		static long long GetTickTime();

		/// <summary>
		/// 获取相对时间戳，自系统启动以来到现在经过的微秒
		/// </summary>
		/// <returns></returns>
		static long long GetTickMicroTime();

		/// <summary>
		/// 获取真实时间戳，从1970年1月1日0时0分0秒开始，到系统现在时间经过的毫秒
		/// </summary>
		/// <returns></returns>
		static long long GetTickTimeReal();

		/// <summary>
		/// 获取当前程序所在目录
		/// </summary>
		/// <returns></returns>
		static std::string GetAppPath();

		/// <summary>
		/// 获取路径的全路径
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static std::string GetFullPath(const std::string& path);

		/// <summary>
		/// 获取cpu核心数量
		/// </summary>
		/// <returns></returns>
		static int GetCpuCount();

		/// <summary>
		/// 判断文件夹是否存在
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static bool IsHaveDirectory(const std::string& path);

		/// <summary>
		/// 判断文件是否存在
		/// </summary>
		/// <param name="file_path"></param>
		/// <returns></returns>
		static bool IsHaveFile(const std::string& file_path);

		/// <summary>
		/// 创建文件夹
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static bool CreateDirectory(std::string path);

		/// <summary>
		/// 删除文件夹
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static bool RemoveDirectory(std::string path);

		/// <summary>
		/// 删除文件
		/// </summary>
		/// <param name="file_path"></param>
		/// <returns></returns>
		static bool RemoveFile(std::string file_path);

		/// <summary>
		/// 读取文本文件
		/// </summary>
		/// <param name="file_path"></param>
		/// <returns></returns>
		static std::string ReadTxtFile(std::string file_path);

		typedef struct cpu_occupy_
		{
			char name[20];
			unsigned long long user;
			unsigned long long nice;
			unsigned long long system;
			unsigned long long idle;
			unsigned long long iowait;
			unsigned long long irq;
			unsigned long long softirq;
			unsigned long long stealstolen;
			unsigned long long guest;
		}cpu_occupy_t;

		/// <summary>
		/// 获取cpu状态
		/// </summary>
		/// <param name="status"></param>
		/// <returns></returns>
		static bool GetCpuStatus(cpu_occupy_t& status);

		/// <summary>
		/// 获取cpu使用率
		/// </summary>
		/// <param name="tick"></param>
		/// <returns></returns>
		static float GetCpuUsage(long long tick = 100);

		/// <summary>
		/// 获取内存使用率
		/// </summary>
		/// <returns></returns>
		static float GetMemoryUsage();

		/// <summary>
		/// 子网掩码位数转字符串
		/// 17 -> 255.255.128.0
		/// </summary>
		/// <param name="bit"></param>
		/// <returns></returns>
		static std::string BitToNetMask(int bit);

		/// <summary>
		/// 子网掩码转位数
		/// 255.255.128.0 -> 17
		/// </summary>
		/// <param name="str_mask"></param>
		/// <returns></returns>
		static int NetMaskToBit(std::string str_mask);

		/// <summary>
		/// 磁盘信息
		/// </summary>
		typedef struct DiskInfo
		{
			std::string						uuid = "";				// 设备id
			std::string						name = "";				// 设备名称
			std::string						serial = "";			// 设备序列号
			std::string						maj_min = "";			// 设备的主设备号和次设备号
			std::string						fstype = "";			// 设备的文件系统类型
			std::string						mountpoint = "";		// 设备的挂载点 /home/usr/disk
			bool							ro = false;				// 是否为只读
			bool							rm = true;				// 是否为可移动设备
			long long						size = 0;				// 设备大小（字节）
			std::string						type = "";				// 设备类别
			std::vector<DiskInfo>			children;				// 子分区
		}DiskInfo;

		/// <summary>
		/// 获取磁盘信息
		/// </summary>
		/// <param name="disks"></param>
		/// <param name="sudo_pass"></param>
		/// <returns></returns>
		static bool GetAllDiskInfo(std::vector<DiskInfo>& disks);
		static bool GetDiskInfoWithName(DiskInfo& info, std::string dev_name);
		static bool GetDiskInfoWithSn(DiskInfo& info, std::string dev_sn);
		static bool GetSDDiskInfo(std::vector<DiskInfo>& disks);

		/// <summary>
		/// 获取磁盘实时读写信息
		/// </summary>
		/// <param name="disk_name"></param>
		/// <param name="write_speed"></param>
		/// <param name="read_speed"></param>
		/// <returns></returns>
		static bool GetDiskUseInfo(std::string disk_name, long long& write_speed, long long& read_speed);

		/// <summary>
		/// 获取指定磁盘当前读写速度
		/// </summary>
		/// <param name="disk_name"></param>
		/// <param name="write_speed"></param>
		/// <param name="read_speed"></param>
		/// <returns></returns>
		static bool GetDiskUseStatus(std::string disk_name, long long& write_speed, long long& read_speed);

		/// <summary>
		/// 获取所有磁盘当前读写速度
		/// </summary>
		/// <param name="write_byte"></param>
		/// <param name="read_byte"></param>
		/// <returns></returns>
		static bool GetDiskUseStatus(long long& write_byte, long long& read_byte);

		/// <summary>
		/// 挂载磁盘
		/// </summary>
		/// <param name="disk_sn"></param>
		/// <param name="path"></param>
		/// <returns></returns>
		static bool MountDisk_SN(std::string disk_sn, std::string path);
		static bool MountDisk_Name(std::string device_name, std::string path);
		
		/// <summary>
		/// 卸载磁盘
		/// </summary>
		/// <param name="disk_sn"></param>
		/// <param name="sudo_pass"></param>
		/// <returns></returns>
		static bool UMountDisk_SN(std::string disk_sn);
		static bool UMountDisk_Name(std::string device_name);

		/// <summary>
		/// 获取路径存储空间信息（字节）
		/// </summary>
		/// <param name="path"></param>
		/// <param name="size"></param>
		/// <param name="used"></param>
		/// <param name="free"></param>
		/// <returns></returns>
		static bool GetPathSizeInfo(std::string path, long long& size, long long& used, long long& free);
		
		/// <summary>
		/// 文件信息
		/// </summary>
		typedef struct FileInfo
		{
			/// <summary>
			/// 文件名称
			/// </summary>
			std::string				name = "";
			/// <summary>
			/// 扩展名
			/// </summary>
			std::string				ext_name = "";
			/// <summary>
			/// 文件创建时间
			/// </summary>
			cxDateTime				createtime;
			/// <summary>
			/// 文件最后访问时间
			/// </summary>
			cxDateTime				lastaccesstime;
			/// <summary>
			/// 文件最后改动时间
			/// </summary>
			cxDateTime				lastwritetime;
			/// <summary>
			/// 文件大小（字节）
			/// </summary>
			long long				filesize = 0;
		}FileInfo;

		/// <summary>
		/// 获取文件信息
		/// </summary>
		/// <param name="path"></param>
		/// <param name="info"></param>
		/// <returns></returns>
		static bool GetFileInfo(std::string path, FileInfo& info);

		/// <summary>
		/// 获取文件大小（字节）
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		static long long GetFileSize(const std::string& path);

		/// <summary>
		/// 获取目录下的所有文件
		/// </summary>
		/// <param name="path"></param>
		/// <param name="infos"></param>
		static void GetFiles(std::string path, std::vector<FileInfo>& infos);

		/// <summary>
		/// 获取路径下的所有文件夹
		/// </summary>
		/// <param name="path"></param>
		/// <param name="dirs"></param>
		static void GetDirectorys(std::string path, std::vector<std::string>& dirs);

		/// <summary>
		/// 创建随机数字
		/// </summary>
		/// <param name="min"></param>
		/// <param name="max"></param>
		/// <returns></returns>
		static int CreateRandNum(int min, int max);

		/// <summary>
		/// 创建随机字符串
		/// </summary>
		/// <param name="length"></param>
		/// <param name="seed"></param>
		/// <returns></returns>
		static std::string CreateRandString(unsigned int length, std::string seed);

		/// <summary>
		/// sha加密
		/// </summary>
		/// <param name="buf"></param>
		/// <returns></returns>
		static std::vector<char> Sha1(std::vector<char>& buf);

		/// <summary>
		/// base64编码
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		static std::string Base64_Encodec(std::vector<char>& src);
		static std::string Base64_Encodec(char* p, unsigned int size);

		/// <summary>
		/// 创建token
		/// </summary>
		/// <returns></returns>
		static std::string CreateToken();

		/// <summary>
		/// 创建guid
		/// </summary>
		/// <returns></returns>
		static std::string CreateGuid();

		/// <summary>
		/// 获取进程id
		/// </summary>
		/// <returns></returns>
		static unsigned long long GetProcessId();

		typedef struct NetDeviceInfo
		{
			std::string				name = "";					// 设备名称
			std::string				connect_name = "";			// 连接名
			bool					connected = false;			// 连接是否已连接上
			std::string				ip_mode = "";				// 连接模式
			std::string				ip = "";					// ip地址
			std::string				netmask = "";				// 子网掩码
			std::string				gateway = "";				// 网关
			std::string				broadcast = "";				// 广播地址
			std::string				dns = "";					// dns地址
		}NetDeviceInfo;
		/// <summary>
		/// 获取网卡信息
		/// </summary>
		/// <param name="infos"></param>
		/// <param name="device_name"></param>
		/// <returns></returns>
		static bool GetNetDeviceInfo(std::vector<NetDeviceInfo>& infos, std::string device_name = "");

		typedef struct NetDeviceUseInfo
		{
			std::string				name = "";					// 设备名称
			long long				send_bytes = 0;				// 发送数据量
			long long				recv_bytes = 0;				// 接收数据量
			int						speed = 0;					// 连接速度
		}NetDeviceUseInfo;
		/// <summary>
		/// 获取网络设备使用信息
		/// </summary>
		/// <param name="infos"></param>
		/// <param name="device_name"></param>
		/// <returns></returns>
		static bool GetNetDeviceUseInfo(NetDeviceUseInfo& info, std::string device_name);

		/// <summary>
		/// 获取网络使用状态
		/// </summary>
		/// <returns></returns>
		static bool GetNetUseStatus(float& use_status, long long& speed, long long& send_speed, long long& recv_speed);

		static bool GetNetUseStatus(long long& speed, long long& send_byte, long long& recv_byte);

		/// <summary>
		/// 获取网络设备使用状态
		/// </summary>
		static bool GetNetDeviceUseStatus(std::string device_name, unsigned int time, int& speed, long long& send_bytes, long long& recv_bytes);

		/// <summary>
		/// 获取jetson盒子的gpu使用状态
		/// </summary>
		/// <returns></returns>
		static float GetJetsonGpuUseStatus();

		/// <summary>
		/// 获取jetson盒子的风扇转速
		/// </summary>
		/// <returns></returns>
		static int GetJetsonFanSpeed();

		typedef struct JetsonTemperature
		{
			int				cpu_therm = 0;				// cpu温度
			int				gpu_therm = 0;				// gpu温度
			int				aux_therm = 0;				// 主板温度
			int				ao_therm = 0;				// 电源温度
			int				pmic_die = 0;
			int				thermal_fan_est = 0;		// 风扇温度
		}JetsonTemperature;
		static bool GetJetsonTemperature(JetsonTemperature& info);

		/// <summary>
		/// md5加密
		/// </summary>
		/// <param name="str"></param>
		/// <returns></returns>
		static std::string Md5(std::string str);
	};
}
