#include <signal.h>
#include "cx.h"
#include "cs_server.h"
#include "cs_httpserver.h"

std::shared_ptr<cx::cxSemaphore> GSEM;
struct sigaction GACT;
//std::mutex GMUTEX;
//bool GSRSSTOP = false;

void handle_signal(int n)
{
	GSEM->ReleaseOne();
}
void Add_OnAppExit()
{
	GSEM = std::make_shared<cx::cxSemaphore>();

	signal(SIGHUP, handle_signal); // 用户终端关闭
	signal(SIGINT, handle_signal); // ctrl + c
	signal(SIGQUIT, handle_signal); // 程序出错
	signal(SIGTERM, handle_signal); // kill
	signal(SIGABRT, handle_signal); // 调用了abort
}

cx::cxConfigFile							G_Config;

void SaveNowPid()
{
	std::ofstream fs(cx::cxCommonFun::GetAppPath() + "/cs.pid");
	if (!fs)
	{
		return;
	}

	fs << cx::cxCommonFun::GetProcessId();
	fs.close();
}

int main(int argc, char** argv)
{
	cxLogerEnable("系统开始启动");

	SaveNowPid();
	Add_OnAppExit();

	cx::cxThreadPool::Init();

	if (!G_Config.LoadConfig(cx::cxCommonFun::GetAppPath() + "/cs.conf"))
	{
		cxLogerError("系统启动失败，配置文件读取失败");
		goto END;
	}

	cxLogerEnable("配置文件读取成功");

	//if (!csServer::StartService(G_Config.GetValue<std::string>("ip"), G_Config.GetValue<int>("port")))
	//{
	//	cxLogerError("系统启动失败，服务器socket创建失败");
	//	goto END;
	//}

	if (!csHttpServer::StartService(G_Config.GetValue<std::string>("ip"), G_Config.GetValue<int>("port")))
	{
		cxLogerError("系统启动失败，服务器socket创建失败");
		goto END;
	}

	cxLogerEnable("系统启动成功");
	GSEM->WaitOne();
END:
	cxLogerEnable("系统关闭。。。");
	csServer::StopService();
    return 0;
}