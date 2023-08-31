#include "cx_http_controller.h"
#include <fstream>

namespace cx
{
	cxWebController::cxWebController()
	{

	}
	cxWebController::~cxWebController()
	{

	}
	std::string cxWebController::GetPath()
	{
		return m_path;
	}
	bool cxWebController::IsWebSocket()
	{
		return m_bws;
	}
	bool cxWebController::GetCross()
	{
		return m_bcross;
	}
	void cxWebController::GET(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		_GET_HEAD_(request, response, true);
	}
	void cxWebController::HEAD(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		_GET_HEAD_(request, response, false);
	}
	void cxWebController::POST(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		if (request->GetHeader<std::string>("Content-Type").find("application/json") >= 0)
		{
			Json::Value json_req, json_res;
			std::shared_ptr<std::vector<char>> body;
			if (!request->ReadBodyToEnd(body))
			{
				return;
			}
			if (body != NULL && body->size() > 0)
			{
				std::string str_json(body->begin(), body->end());
				Json::Reader read;
				if (!read.parse(str_json, json_req))
				{
					response->Response(400);
					return;
				}
			}

			POST(json_req, json_res);

			if (!json_res.isNull())
			{
				response->SetHeader("Content-Type", "application/json");
				response->SetHeader("Content-Length", std::to_string(json_res.toStyledString().size()));
				response->Response(200);
				response->SendBody(json_res);
			}
			else
			{
				response->Response(200);
			}
		}
		else
		{
			response->Response(405);
		}
	}
	void cxWebController::POST(const Json::Value& request, Json::Value& response)
	{

	}
	void cxWebController::OPTIONS(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		if (m_bcross)
		{
			response->Response(200);
		}
		else
		{
			response->Response(405);
		}
	}
	void cxWebController::PUT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		response->Response(405);
	}
	void cxWebController::DELETE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		response->Response(405);
	}
	void cxWebController::TRACE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		response->Response(405);
	}
	void cxWebController::CONNECT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		response->Response(405);
	}
	void cxWebController::WS_OnAccept(cxHttpStream::Ptr stream)
	{

	}
	void cxWebController::WS_OnRecvBin(cxHttpStream::Ptr stream, void* data, unsigned int size)
	{

	}
	void cxWebController::WS_OnRecvStr(cxHttpStream::Ptr stream, const std::string& data)
	{

	}
	void cxWebController::WS_OnPing(cxHttpStream::Ptr stream)
	{
		stream->WS_Pong();
	}
	void cxWebController::WS_OnPong(cxHttpStream::Ptr stream)
	{

	}
	void cxWebController::WS_OnClose(cxHttpStream::Ptr stream)
	{
		stream->Close();
	}
	bool cxWebController::ParseRequestFile(cxHttpRequest::Ptr request, std::string& file_path, std::string& file_ext)
	{
		if (request->GetUrl() != "/")
		{
			file_path = m_path + request->GetUrl();
			if (!cxCommonFun::IsHaveFile(file_path))
			{
				return false;
			}
			int index = file_path.find_last_of(".");
			if (index > 0)
			{
				file_ext = file_path.substr(index + 1);
			}
		}
		else
		{
			if (cxCommonFun::IsHaveFile(m_path + "/index.html"))
			{
				file_path = m_path + "/index.html";
			}
			else if (cxCommonFun::IsHaveFile(m_path + "/index.htm"))
			{
				file_path = m_path + "/index.htm";
			}
			else
			{
				return false;
			}
			int index = file_path.find_last_of(".");
			if (index > 0)
			{
				file_ext = file_path.substr(index + 1);
			}
		}

		return true;
	}
	void cxWebController::_GET_HEAD_(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response, bool bget)
	{
		if (m_path == "")
		{
			response->Response(405);
			return;
		}

		std::string file_path = "";
		std::string file_ext = "";

		if (!ParseRequestFile(request, file_path, file_ext))
		{
			response->Response(404);
			return;
		}

		long long file_size = cxCommonFun::GetFileSize(file_path);
		if (file_size <= 0)
		{
			response->Response(404);
			return;
		}

		std::string content_type = GetContentType(file_ext);

		response->SetHeader("Content-Type", content_type);
		response->SetHeader("Content-Length", std::to_string(file_size));

		if (!bget)
		{
			response->Response(200);
			return;
		}

		std::ifstream file(file_path, std::ios::binary);
		if (!file.is_open())
		{
			response->Response(500);
			return;
		}
		response->Response(200);
		response->SendBody(file, file_size);
		file.close();
	}
	std::string cxWebController::GetContentType(std::string ext)
	{
		if (ext == "1") return "application/x-001";
		if (ext == "301") return "application/x-301";
		if (ext == "323") return "text/h323";
		if (ext == "906") return "application/x-906";
		if (ext == "907") return "drawing/907";
		if (ext == "a11") return "application/x-a11";
		if (ext == "acp") return "audio/x-mei-aac";
		if (ext == "ai") return "application/postscript";
		if (ext == "aif") return "audio/aiff";
		if (ext == "aifc") return "audio/aiff";
		if (ext == "aiff") return "audio/aiff";
		if (ext == "anv") return "application/x-anv";
		if (ext == "asa") return "text/asa";
		if (ext == "asf") return "video/x-ms-asf";
		if (ext == "asp") return "text/asp";
		if (ext == "asx") return "video/x-ms-asf";
		if (ext == "au") return "audio/basic";
		if (ext == "avi") return "video/avi";
		if (ext == "awf") return "application/vnd.adobe.workflow";
		if (ext == "biz") return "text/xml";
		if (ext == "bmp") return "application/x-bmp";
		if (ext == "bot") return "application/x-bot";
		if (ext == "c4t") return "application/x-c4t";
		if (ext == "c90") return "application/x-c90";
		if (ext == "cal") return "application/x-cals";
		if (ext == "cat") return "application/vnd.ms-pki.seccat";
		if (ext == "cdf") return "application/x-netcdf";
		if (ext == "cdr") return "application/x-cdr";
		if (ext == "cel") return "application/x-cel";
		if (ext == "cer") return "application/x-x509-ca-cert";
		if (ext == "cg4") return "application/x-g4";
		if (ext == "cgm") return "application/x-cgm";
		if (ext == "cit") return "application/x-cit";
		if (ext == "class") return "java/*";
		if (ext == "cml") return "text/xml";
		if (ext == "cmp") return "application/x-cmp";
		if (ext == "cmx") return "application/x-cmx";
		if (ext == "cot") return "application/x-cot";
		if (ext == "crl") return "application/pkix-crl";
		if (ext == "crt") return "application/x-x509-ca-cert";
		if (ext == "csi") return "application/x-csi";
		if (ext == "css") return "text/css";
		if (ext == "cut") return "application/x-cut";
		if (ext == "dbf") return "application/x-dbf";
		if (ext == "dbm") return "application/x-dbm";
		if (ext == "dbx") return "application/x-dbx";
		if (ext == "dcd") return "text/xml";
		if (ext == "dcx") return "application/x-dcx";
		if (ext == "der") return "application/x-x509-ca-cert";
		if (ext == "dgn") return "application/x-dgn";
		if (ext == "dib") return "application/x-dib";
		if (ext == "dll") return "application/x-msdownload";
		if (ext == "doc") return "application/msword";
		if (ext == "dot") return "application/msword";
		if (ext == "drw") return "application/x-drw";
		if (ext == "dtd") return "text/xml";
		if (ext == "dwf") return "Model/vnd.dwf";
		if (ext == "dwf") return "application/x-dwf";
		if (ext == "dwg") return "application/x-dwg";
		if (ext == "dxb") return "application/x-dxb";
		if (ext == "dxf") return "application/x-dxf";
		if (ext == "edn") return "application/vnd.adobe.edn";
		if (ext == "emf") return "application/x-emf";
		if (ext == "eml") return "message/rfc822";
		if (ext == "ent") return "text/xml";
		if (ext == "epi") return "application/x-epi";
		if (ext == "eps") return "application/x-ps";
		if (ext == "eps") return "application/postscript";
		if (ext == "etd") return "application/x-ebx";
		if (ext == "exe") return "application/x-msdownload";
		if (ext == "fax") return "image/fax";
		if (ext == "fdf") return "application/vnd.fdf";
		if (ext == "fif") return "application/fractals";
		if (ext == "fo") return "text/xml";
		if (ext == "frm") return "application/x-frm";
		if (ext == "g4") return "application/x-g4";
		if (ext == "gbr") return "application/x-gbr";
		if (ext == "gcd") return "application/x-gcd";
		if (ext == "gif") return "image/gif";
		if (ext == "gl2") return "application/x-gl2";
		if (ext == "gp4") return "application/x-gp4";
		if (ext == "hgl") return "application/x-hgl";
		if (ext == "hmr") return "application/x-hmr";
		if (ext == "hpg") return "application/x-hpgl";
		if (ext == "hpl") return "application/x-hpl";
		if (ext == "hqx") return "application/mac-binhex40";
		if (ext == "hrf") return "application/x-hrf";
		if (ext == "hta") return "application/hta";
		if (ext == "htc") return "text/x-component";
		if (ext == "htm") return "text/html";
		if (ext == "html") return "text/html";
		if (ext == "htt") return "text/webviewhtml";
		if (ext == "htx") return "text/html";
		if (ext == "icb") return "application/x-icb";
		if (ext == "ico") return "image/x-icon";
		if (ext == "ico") return "application/x-ico";
		if (ext == "iff") return "application/x-iff";
		if (ext == "ig4") return "application/x-g4";
		if (ext == "igs") return "application/x-igs";
		if (ext == "iii") return "application/x-iphone";
		if (ext == "img") return "application/x-img";
		if (ext == "ins") return "application/x-internet-signup";
		if (ext == "isp") return "application/x-internet-signup";
		if (ext == "IVF") return "video/x-ivf";
		if (ext == "java") return "java/*";
		if (ext == "jfif") return "image/jpeg";
		if (ext == "jpe") return "image/jpeg";
		if (ext == "jpe") return "application/x-jpe";
		if (ext == "jpeg") return "image/jpeg";
		if (ext == "jpg") return "image/jpeg";
		//if (ext == "jpg") return "application/x-jpg";
		if (ext == "js") return "application/x-javascript";
		if (ext == "jsp") return "text/html";
		if (ext == "la1") return "audio/x-liquid-file";
		if (ext == "lar") return "application/x-laplayer-reg";
		if (ext == "latex") return "application/x-latex";
		if (ext == "lavs") return "audio/x-liquid-secure";
		if (ext == "lbm") return "application/x-lbm";
		if (ext == "lmsff") return "audio/x-la-lms";
		if (ext == "ls") return "application/x-javascript";
		if (ext == "ltr") return "application/x-ltr";
		if (ext == "m1v") return "video/x-mpeg";
		if (ext == "m2v") return "video/x-mpeg";
		if (ext == "m3u") return "audio/mpegurl";
		if (ext == "m3u8") return "application/vnd.apple.mpegurl";
		if (ext == "m4a") return "audio/mp4";
		if (ext == "m4b") return "audio/mp4";
		if (ext == "m4r") return "audio/mp4";
		if (ext == "m4e") return "video/mpeg4";
		if (ext == "mac") return "application/x-mac";
		if (ext == "man") return "application/x-troff-man";
		if (ext == "math") return "text/xml";
		if (ext == "mdb") return "application/msaccess";
		//if (ext == "mdb") return "application/x-mdb";
		if (ext == "mfp") return "application/x-shockwave-flash";
		if (ext == "mht") return "message/rfc822";
		if (ext == "mhtml") return "message/rfc822";
		if (ext == "mi") return "application/x-mi";
		if (ext == "mid") return "audio/mid";
		if (ext == "midi") return "audio/mid";
		if (ext == "mil") return "application/x-mil";
		if (ext == "mml") return "text/xml";
		if (ext == "mnd") return "audio/x-musicnet-download";
		if (ext == "mns") return "audio/x-musicnet-stream";
		if (ext == "mocha") return "application/x-javascript";
		if (ext == "movie") return "video/x-sgi-movie";
		if (ext == "mp1") return "audio/mp1";
		if (ext == "mp2") return "audio/mp2";
		if (ext == "mp2v") return "video/mpeg";
		if (ext == "mp3") return "audio/mp3";
		if (ext == "mp4") return "video/mpeg4";
		if (ext == "mpa") return "video/x-mpg";
		if (ext == "mpd") return "application/vnd.ms-project";
		if (ext == "mpe") return "video/x-mpeg";
		if (ext == "mpeg") return "video/mpg";
		if (ext == "mpg") return "video/mpg";
		if (ext == "mpga") return "audio/rn-mpeg";
		if (ext == "mpp") return "application/vnd.ms-project";
		if (ext == "mps") return "video/x-mpeg";
		if (ext == "mpt") return "application/vnd.ms-project";
		if (ext == "mpv") return "video/mpg";
		if (ext == "mpv2") return "video/mpeg";
		if (ext == "mpw") return "application/vnd.ms-project";
		if (ext == "mpx") return "application/vnd.ms-project";
		if (ext == "mtx") return "text/xml";
		if (ext == "mxp") return "application/x-mmxp";
		if (ext == "net") return "image/pnetvue";
		if (ext == "nrf") return "application/x-nrf";
		if (ext == "nws") return "message/rfc822";
		if (ext == "odc") return "text/x-ms-odc";
		if (ext == "out") return "application/x-out";
		if (ext == "p10") return "application/pkcs10";
		if (ext == "p12") return "application/x-pkcs12";
		if (ext == "p7b") return "application/x-pkcs7-certificates";
		if (ext == "p7c") return "application/pkcs7-mime";
		if (ext == "p7m") return "application/pkcs7-mime";
		if (ext == "p7r") return "application/x-pkcs7-certreqresp";
		if (ext == "p7s") return "application/pkcs7-signature";
		if (ext == "pc5") return "application/x-pc5";
		if (ext == "pci") return "application/x-pci";
		if (ext == "pcl") return "application/x-pcl";
		if (ext == "pcx") return "application/x-pcx";
		if (ext == "pdf") return "application/pdf";
		if (ext == "pdf") return "application/pdf";
		if (ext == "pdx") return "application/vnd.adobe.pdx";
		if (ext == "pfx") return "application/x-pkcs12";
		if (ext == "pgl") return "application/x-pgl";
		if (ext == "pic") return "application/x-pic";
		if (ext == "pko") return "application/vnd.ms-pki.pko";
		if (ext == "pl") return "application/x-perl";
		if (ext == "plg") return "text/html";
		if (ext == "pls") return "audio/scpls";
		if (ext == "plt") return "application/x-plt";
		if (ext == "png") return "image/png";
		if (ext == "png") return "application/x-png";
		if (ext == "pot") return "application/vnd.ms-powerpoint";
		if (ext == "ppa") return "application/vnd.ms-powerpoint";
		if (ext == "ppm") return "application/x-ppm";
		if (ext == "pps") return "application/vnd.ms-powerpoint";
		if (ext == "ppt") return "application/vnd.ms-powerpoint";
		if (ext == "ppt") return "application/x-ppt";
		if (ext == "pr") return "application/x-pr";
		if (ext == "prf") return "application/pics-rules";
		if (ext == "prn") return "application/x-prn";
		if (ext == "prt") return "application/x-prt";
		if (ext == "ps") return "application/x-ps";
		if (ext == "ps") return "application/postscript";
		if (ext == "ptn") return "application/x-ptn";
		if (ext == "pwz") return "application/vnd.ms-powerpoint";
		if (ext == "r3t") return "text/vnd.rn-realtext3d";
		if (ext == "ra") return "audio/vnd.rn-realaudio";
		if (ext == "ram") return "audio/x-pn-realaudio";
		if (ext == "ras") return "application/x-ras";
		if (ext == "rat") return "application/rat-file";
		if (ext == "rdf") return "text/xml";
		if (ext == "rec") return "application/vnd.rn-recording";
		if (ext == "red") return "application/x-red";
		if (ext == "rgb") return "application/x-rgb";
		if (ext == "rjs") return "application/vnd.rn-realsystem-rjs";
		if (ext == "rjt") return "application/vnd.rn-realsystem-rjt";
		if (ext == "rlc") return "application/x-rlc";
		if (ext == "rle") return "application/x-rle";
		if (ext == "rm") return "application/vnd.rn-realmedia";
		if (ext == "rmf") return "application/vnd.adobe.rmf";
		if (ext == "rmi") return "audio/mid";
		if (ext == "rmj") return "application/vnd.rn-realsystem-rmj";
		if (ext == "rmm") return "audio/x-pn-realaudio";
		if (ext == "rmp") return "application/vnd.rn-rn_music_package";
		if (ext == "rms") return "application/vnd.rn-realmedia-secure";
		if (ext == "rmvb") return "application/vnd.rn-realmedia-vbr";
		if (ext == "rmx") return "application/vnd.rn-realsystem-rmx";
		if (ext == "rnx") return "application/vnd.rn-realplayer";
		if (ext == "rp") return "image/vnd.rn-realpix";
		if (ext == "rpm") return "audio/x-pn-realaudio-plugin";
		if (ext == "rsml") return "application/vnd.rn-rsml";
		if (ext == "rt") return "text/vnd.rn-realtext";
		if (ext == "rtf") return "application/msword";
		if (ext == "rtf") return "application/x-rtf";
		if (ext == "rv") return "video/vnd.rn-realvideo";
		if (ext == "sam") return "application/x-sam";
		if (ext == "sat") return "application/x-sat";
		if (ext == "sdp") return "application/sdp";
		if (ext == "sdw") return "application/x-sdw";
		if (ext == "sit") return "application/x-stuffit";
		if (ext == "slb") return "application/x-slb";
		if (ext == "sld") return "application/x-sld";
		if (ext == "slk") return "drawing/x-slk";
		if (ext == "smi") return "application/smil";
		if (ext == "smil") return "application/smil";
		if (ext == "smk") return "application/x-smk";
		if (ext == "snd") return "audio/basic";
		if (ext == "sol") return "text/plain";
		if (ext == "sor") return "text/plain";
		if (ext == "spc") return "application/x-pkcs7-certificates";
		if (ext == "spl") return "application/futuresplash";
		if (ext == "spp") return "text/xml";
		if (ext == "ssm") return "application/streamingmedia";
		if (ext == "sst") return "application/vnd.ms-pki.certstore";
		if (ext == "stl") return "application/vnd.ms-pki.stl";
		if (ext == "stm") return "text/html";
		if (ext == "sty") return "application/x-sty";
		if (ext == "svg") return "text/xml";
		if (ext == "swf") return "application/x-shockwave-flash";
		if (ext == "tdf") return "application/x-tdf";
		if (ext == "tg4") return "application/x-tg4";
		if (ext == "tga") return "application/x-tga";
		if (ext == "tif") return "image/tiff";
		if (ext == "tif") return "application/x-tif";
		if (ext == "tiff") return "image/tiff";
		if (ext == "tld") return "text/xml";
		if (ext == "top") return "drawing/x-top";
		if (ext == "torrent") return "application/x-bittorrent";
		if (ext == "ts") return "video/mp2t";
		if (ext == "tsd") return "text/xml";
		if (ext == "txt") return "text/plain";
		if (ext == "uin") return "application/x-icq";
		if (ext == "uls") return "text/iuls";
		if (ext == "vcf") return "text/x-vcard";
		if (ext == "vda") return "application/x-vda";
		if (ext == "vdx") return "application/vnd.visio";
		if (ext == "vml") return "text/xml";
		if (ext == "vpg") return "application/x-vpeg005";
		if (ext == "vsd") return "application/vnd.visio";
		if (ext == "vsd") return "application/x-vsd";
		if (ext == "vss") return "application/vnd.visio";
		if (ext == "vst") return "application/vnd.visio";
		if (ext == "vst") return "application/x-vst";
		if (ext == "vsw") return "application/vnd.visio";
		if (ext == "vsx") return "application/vnd.visio";
		if (ext == "vtx") return "application/vnd.visio";
		if (ext == "vxml") return "text/xml";
		if (ext == "wav") return "audio/wav";
		if (ext == "wax") return "audio/x-ms-wax";
		if (ext == "wb1") return "application/x-wb1";
		if (ext == "wb2") return "application/x-wb2";
		if (ext == "wb3") return "application/x-wb3";
		if (ext == "wbmp") return "image/vnd.wap.wbmp";
		if (ext == "wiz") return "application/msword";
		if (ext == "wk3") return "application/x-wk3";
		if (ext == "wk4") return "application/x-wk4";
		if (ext == "wkq") return "application/x-wkq";
		if (ext == "wks") return "application/x-wks";
		if (ext == "wm") return "video/x-ms-wm";
		if (ext == "wma") return "audio/x-ms-wma";
		if (ext == "wmd") return "application/x-ms-wmd";
		if (ext == "wmf") return "application/x-wmf";
		if (ext == "wml") return "text/vnd.wap.wml";
		if (ext == "wmv") return "video/x-ms-wmv";
		if (ext == "wmx") return "video/x-ms-wmx";
		if (ext == "wmz") return "application/x-ms-wmz";
		if (ext == "wp6") return "application/x-wp6";
		if (ext == "wpd") return "application/x-wpd";
		if (ext == "wpg") return "application/x-wpg";
		if (ext == "wpl") return "application/vnd.ms-wpl";
		if (ext == "wq1") return "application/x-wq1";
		if (ext == "wr1") return "application/x-wr1";
		if (ext == "wri") return "application/x-wri";
		if (ext == "wrk") return "application/x-wrk";
		if (ext == "ws") return "application/x-ws";
		if (ext == "ws2") return "application/x-ws";
		if (ext == "wsc") return "text/scriptlet";
		if (ext == "wsdl") return "text/xml";
		if (ext == "wvx") return "video/x-ms-wvx";
		if (ext == "xdp") return "application/vnd.adobe.xdp";
		if (ext == "xdr") return "text/xml";
		if (ext == "xfd") return "application/vnd.adobe.xfd";
		if (ext == "xfdf") return "application/vnd.adobe.xfdf";
		if (ext == "xhtml") return "text/html";
		if (ext == "xls") return "application/vnd.ms-excel";
		if (ext == "xls") return "application/x-xls";
		if (ext == "xlw") return "application/x-xlw";
		if (ext == "xml") return "text/xml";
		if (ext == "xpl") return "audio/scpls";
		if (ext == "xq") return "text/xml";
		if (ext == "xql") return "text/xml";
		if (ext == "xquery") return "text/xml";
		if (ext == "xsd") return "text/xml";
		if (ext == "xsl") return "text/xml";
		if (ext == "xslt") return "text/xml";
		if (ext == "xwd") return "application/x-xwd";
		if (ext == "x_b") return "application/x-x_b";
		if (ext == "x_t") return "application/x-x_t";
		return "application/octet-stream";
	}

	cxHttpController::cxHttpController()
	{

	}
	cxHttpController::cxHttpController(std::string path)
	{
		m_path = path;
	}
	cxHttpController::cxHttpController(bool bcross)
	{
		m_bcross = bcross;
	}
	cxHttpController::cxHttpController(bool bcross, std::string path)
	{
		m_bcross = bcross;
		m_path = path;
	}
	cxHttpController::~cxHttpController()
	{

	}
	std::string cxHttpController::GetPath()
	{
		return m_path;
	}
	bool cxHttpController::IsWebSocket()
	{
		return m_bws;
	}
	void cxHttpController::GET(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::GET(request, response);
	}
	void cxHttpController::HEAD(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::HEAD(request, response);
	}
	void cxHttpController::POST(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::POST(request, response);
	}
	void cxHttpController::POST(const Json::Value& request, Json::Value& response)
	{
		cxWebController::POST(request, response);
	}
	void cxHttpController::OPTIONS(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::OPTIONS(request, response);
	}
	void cxHttpController::PUT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::PUT(request, response);
	}
	void cxHttpController::DELETE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::DELETE(request, response);
	}
	void cxHttpController::TRACE(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::TRACE(request, response);
	}
	void cxHttpController::CONNECT(cxHttpRequest::Ptr request, cxHttpResponse::Ptr response)
	{
		cxWebController::CONNECT(request, response);
	}

	cxWebSocketController::cxWebSocketController()
	{
		m_bws = true;
	}
	cxWebSocketController::cxWebSocketController(bool bcross)
	{
		m_bcross = true;
	}
	cxWebSocketController::~cxWebSocketController()
	{

	}
	void cxWebSocketController::WS_OnAccept(cxHttpStream::Ptr stream)
	{
		cxWebController::WS_OnAccept(stream);
	}
	void cxWebSocketController::WS_OnRecvBin(cxHttpStream::Ptr stream, void* data, unsigned int size)
	{
		cxWebController::WS_OnRecvBin(stream, data, size);
	}
	void cxWebSocketController::WS_OnRecvStr(cxHttpStream::Ptr stream, const std::string& data)
	{
		cxWebController::WS_OnRecvStr(stream, data);
	}
	void cxWebSocketController::WS_OnPing(cxHttpStream::Ptr stream)
	{
		cxWebController::WS_OnPing(stream);
	}
	void cxWebSocketController::WS_OnPong(cxHttpStream::Ptr stream)
	{
		cxWebController::WS_OnPong(stream);
	}
	void cxWebSocketController::WS_OnClose(cxHttpStream::Ptr stream)
	{
		cxWebController::WS_OnClose(stream);
	}
}