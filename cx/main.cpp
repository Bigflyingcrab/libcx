#include <iostream>
#include "cx/cx.h"

std::string str = "HTTP/1.1 200 OK\r\nServer: libcx\r\nConnection: close\r\n\r\n";

void OnRecv(cx::cxSocketTcp::Ptr sock, void* data, unsigned int size)
{
	std::string str((char*)data, (char*)data + size);
	std::cout << str << std::endl;


}

void OnClose(cx::cxSocketTcp::Ptr sock)
{
	std::cout << "连接关闭" << std::endl;
}

void OnAccept(cx::cxSocketTcp::Ptr server, cx::cxSocketTcp::Ptr sock)
{
	//std::cout << 11111 << std::endl;
	sock->SetOnClose(OnClose);
	sock->BeginRecv(OnRecv);
	//std::cout << 22222 << std::endl;
}

class TestController : public cx::cxHttpController
{
public:
	void GET(cx::cxHttpRequest::Ptr request, cx::cxHttpResponse::Ptr response)
	{
		//std::this_thread::sleep_for(std::chrono::microseconds(500));
		response->Response(200);
	}
	//void WS_OnAccept(cx::cxHttpStream::Ptr stream)
	//{
	//	std::cout << "WS_OnAccept " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << std::endl;
	//}
	//void WS_OnRecvBin(cx::cxHttpStream::Ptr stream, void* data, unsigned int size)
	//{
	//	std::cout << "WS_OnRecvBin " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << ": sum = " << size << std::endl;
	//}
	//void WS_OnRecvStr(cx::cxHttpStream::Ptr stream, const std::string& data)
	//{
	//	stream->WS_SendData(data);
	//	std::cout << "WS_OnRecvStr " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << ": " << data << std::endl;
	//}
	//void WS_OnPing(cx::cxHttpStream::Ptr stream)
	//{
	//	std::cout << "WS_OnPing " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << std::endl;
	//	cxWebSocketController::WS_OnPing(stream);
	//	stream->WS_Ping();
	//}
	//void WS_OnPong(cx::cxHttpStream::Ptr stream)
	//{
	//	std::cout << "WS_OnPong " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << std::endl;
	//}
	//void WS_OnClose(cx::cxHttpStream::Ptr stream)
	//{
	//	std::cout << "WS_OnClose " << stream->m_socket->GetIp() << ":" << stream->m_socket->GetPort() << std::endl;
	//}
};

class TestMap
{
public:
	TestMap()
	{
		std::cout << "TestMap()" << std::endl;
	}
	TestMap(const TestMap& tm)
	{
		std::cout << "TestMap(const TestMap& tm)" << std::endl;
	}
	~TestMap()
	{
		std::cout << "~TestMap()" << std::endl;
	}
};

#include "cx/camera/camera_control.h"
#include "cx/camera/camera_360star.h"
#include "cx/camera/camera_360anywhere.h"

void Camera_OnClose(CameraControl::Ptr camera)
{
	std::cout << "连接关闭" << std::endl;
}

void Camera_OnAccept(CameraControl::Ptr camera)
{
	std::cout << "相机连入" << std::endl;
	camera->SetOnClose(Camera_OnClose);
}

#include <cmath>
typedef struct Point
{
	bool				bx = false;
	bool				by = false;
	double				tick = 0;
	int					x = 0;
	int					y = 0;
}Point;

class Ballistic
{
private:
	const double					m_g = -98;
private:
	std::vector<Point>				m_points;
	Point							m_postion;
	double							m_angle = 0;
	double							m_speed = 0;
	double							m_wind = 0;
	int								m_height = 0;
	int								m_width = 0;
private:
	double get_tick(double u, double a, double s)
	{
		double temp = u * u + 2 * a * s;
		if (temp < 0)
		{
			return -1;
		}
		double speed = sqrt(temp);
		double t = (speed - u) / a;
		return t;
	}
	int get_dist(double u, double a, double tick)
	{
		double dist = u * tick + (a * tick * tick) / 2.0;
		if (a > 0)
		{ }
		return dist + 0.5;
	}
	void compute()
	{
		// 计算竖直初速度
		double height_speed = m_speed * sin(m_angle);
		// 计算水平初速度
		double horizontal_speed = m_speed * cos(m_angle);
		// 初始化到达最高点时间
		long long height_time = 0;
		// 初始化到达最高点y轴位置
		int hightest = m_postion.y;
		// 竖直初速度大于零，向上开炮
		if (height_speed > 0)
		{
			height_time = height_speed / m_g;
			// 获取炮弹上升y轴每个点位的时间戳
			for (int i = m_postion.y; ; i++)
			{
				double tick = get_tick(height_speed, m_g, i);
				if (tick < 0)
				{
					break;
				}
				Point pt;
				pt.y = m_postion.y + i;
				hightest = pt.y;
				pt.by = true;
				pt.tick = tick;
				m_points.push_back(pt);
			}
		}
		for (int i = 1; i <= hightest; i++)
		{
			double tick = get_tick(0, -1 * m_g, i);
			if (tick < 0)
			{
				break;
			}
			Point pt;
			pt.y = hightest - i;
			pt.by = true;
			pt.tick = tick;
			m_points.push_back(pt);
		}

		for (unsigned int i = 0; i < m_points.size(); i++)
		{
			int x = get_dist(horizontal_speed, m_wind, m_points[i].tick);
			if (x < 0 || x > m_width)
			{
				m_points.erase(m_points.begin() + i, m_points.end());
				break;
			}
			m_points[i].x = x;
			m_points[i].bx = true;
		}
	}
public:
	Ballistic(const Point& postion, double angle, double speed, double wind, int width, int height)
	{
		m_postion = postion;
		m_angle = angle;
		m_speed = speed;
		m_wind = wind;
		m_width = width;
		m_height = height;
		
		compute();		
	}
	~Ballistic()
	{

	}
	bool GetPoint(unsigned long long tick, Point& pt)
	{
		if (m_points.size() == 0)
		{
			return false;
		}
		Point pt1, pt2;
		for (int i = 1; i < m_points.size(); i++)
		{
			if (m_points[i].tick == tick)
			{
				pt = m_points[i];
				return true;
			}
			else if (m_points[i].tick > tick)
			{
				if (i < m_points.size() - 1)
				{
					pt1 = m_points[i];
					pt2 = m_points[i + 1];
					if (abs(pt1.tick - tick) < abs(pt2.tick - tick))
					{
						pt = pt1;
					}
					else
					{
						pt = pt2;
					}
					return true;
				}
				else
				{
					pt = m_points[i];
					return true;
				}
			}
		}
		return true;
	}
	const std::vector<Point>& GetPoints()
	{
		return m_points;
	}
};

int main(int argc, char** argv)
{
	cxLogerEnable("日志" << 123 << 0.25);
	//for (int i = 0; i < 20; i++)
	//{
	//	unsigned long long tick, tick2;
	//	tick = cx::cxCommonFun::GetTickMicroTime();

	//	Point pt;
	//	pt.x = 0;
	//	pt.y = 0;
	//	Ballistic bll(pt, (3.141592654 / 10.0), 300, -500, 640, 480);

	//	tick2 = cx::cxCommonFun::GetTickMicroTime();
	//	std::cout << tick2 - tick << std::endl;
	//}
	//
	//cx::cxCommandProxy::InitConnectInfo("127.0.0.1", 7594);
	//std::vector<LANCameraInfo> cameras;
	//CameraControl::SearchCamera(cameras);
	//CameraControl::StartServer("0.0.0.0", 9000, "/api/login", Camera_OnAccept);
	////cx::cxSafeMap<std::string, std::shared_ptr<TestMap>> map;
	////std::shared_ptr<TestMap> ppp = std::make_shared<TestMap>();
	////std::cout << ppp.use_count() << std::endl;
	////map["1"] = ppp;
	////std::cout << ppp.use_count() << std::endl;
	////std::shared_ptr<TestMap> ptr = map["1"];
	////std::cout << ppp.use_count() << std::endl;
	////map["1"] = NULL;
	////std::cout << ppp.use_count() << std::endl;
	////map["1"] = NULL;
	////cx::cxHttpRequest::Ptr request = std::make_shared<cx::cxHttpRequest>();
	////if (!request->Request(cx::cxHttp_Mode::GET, "http://www.163.com"))
	////{
	////	return 1;
	////}
	////cx::cxHttpResponse::Ptr response = request->GetResponse();
	////if (response->GetCode() != 200)
	////{
	////	return 2;
	////}
	////cx::cxCommandProxy::InitConnectInfo("127.0.0.1", 7594);
	////std::vector<std::string> lines;
	////int ret = cx::cxCommandProxy::Command("lsblk", lines, "");
	////cx::cxCommandProxy::Command("lsblk", lines, "");
	////std::cout << "!23" << std::endl;
	////cx::cxWebSocketClient ws;
	////ws.ConnectServer("ws://192.168.100.5:8080/jsonrpc");
	////Json::Value req;
	////req["jsonrpc"] = "2.0";
	////req["id"] = cx::cxCommonFun::CreateToken();
	////req["method"] = "current_state";
	////Json::Value params;
	////params["infos"];
	////params["infos"].append("versioninfo");
	////req["params"] = params;
	////ws.SendData(req.toStyledString());
	////while (true)
	////{
	////	std::shared_ptr<std::vector<char>> data;
	////	cx::WSDATATYPE type;
	////	if (!ws.Recv(type, data))
	////	{
	////		break;
	////	}

	////	std::string str = "";
	////	if (type == cx::WSDATATYPE::string)
	////	{
	////		str = std::string(data->begin(), data->end());
	////	}

	////	std::cout << str << std::endl;
	////}
	////cx::cxHttpServer::Ptr server = std::make_shared<cx::cxHttpServer>();
	////server->SetWebPath("/usr/share/nginx/html");
	////server->AddController("/api/test", std::make_shared<TestController>());
	////server->StartServer("0.0.0.0", 9090);
	////cx::cxHttpServer::Ptr server2 = std::make_shared<cx::cxHttpServer>();
	////server2->SetWebPath("/home/lmz/players");
	////server2->StartServer("0.0.0.0", 80);
	cx::cxSocketTcp::Ptr socket = cx::cxSocketTcp::CreateSocket();
	if (socket->Connect("192.168.0.98", 5060))
	{
		std::stringstream ss;
		ss << "REGISTER sip:34020000002000000001@3402000000 SIP/2.0" << "\r\n";
		ss << "Via: SIP/2.0/TCP 192.168.5.200:5060;rport;branch=z9hG4bK710869173" << "\r\n";
		ss << "From: <sip:34020000001320000001@3402000000>;tag=1990110079" << "\r\n";
		ss << "To: <sip:34020000001320000001@3402000000>" << "\r\n";
		ss << "Call-ID: 246196434" << "\r\n";
		ss << "CSeq: 1 REGISTER" << "\r\n";
		ss << "Contact: <sip:34020000001320000001@192.168.5.200:5060;transport=TCP;line=a80c1be8c224001>" << "\r\n";
		ss << "Max-Forwards: 70" << "\r\n";
		ss << "User-Agent: TecheAgent" << "\r\n";
		ss << "Expires: 3600" << "\r\n";
		ss << "Content-Length: 0" << "\r\n";
		ss << "\r\n";
		socket->Send((void*)ss.str().c_str(), ss.str().size());
		//socket->Close();

		for (int i = 0; i < 10; i++)
		{
			char buf[65535];
			int len = 0;
			if (socket->Recv(buf, sizeof(buf), len))
			{
				std::string str(buf, buf + len);
				std::cout << str << std::endl;
			}

			ss.str("");
		}

	}

	cxLogerEnable("123" << 1.1 << 'c' << 22222 << "sssssssss");

	socket->Close();
	std::string str;
	std::cin >> str;
	return 0;
}
 