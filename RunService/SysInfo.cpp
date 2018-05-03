// SysInfo.cpp: implementation of the SysInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SysInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SysInfo::SysInfo()
{

}

SysInfo::~SysInfo()
{

}

char* SysInfo::GetSysInfo()
{
	//服务器信息XML格式返回结果
	char server_info[512] = "<Server>\n<annex>\n<srvName string=\"%s\"/>\n<internetip string=\"%s\"/>\n<intranetip string=\"%s\"/>\n<catchTime string=\"%s\"/>\n</annex>\n</Server>";
	
	WSADATA wsaData;
	char name[155]="\0";
	char internetip[32]="\0";
	char intranetip[32]="\0";
	char *ip;
	char datetimebuf[128]="\0";
	char *returnBuf;
	
	CTime tmNow=CTime::GetCurrentTime();
	sprintf(datetimebuf,"%04d-%02d-%02d %02d:%02d:%02d",tmNow.GetYear(),tmNow.GetMonth(),tmNow.GetDay(),tmNow.GetHour(),tmNow.GetMinute(),tmNow.GetSecond());
	
	PHOSTENT hostinfo; 
	if ( WSAStartup( MAKEWORD(2,0), &wsaData ) == 0)	 //
	{ 
		if( gethostname ( name, sizeof(name)) == 0)
		{ 
			if((hostinfo = gethostbyname(name)) != NULL)
			{
				for(int i =0;;i++)
				{
					ip = inet_ntoa (*(struct in_addr *)hostinfo->h_addr_list[i]);
					if (strstr(ip,"192.168"))
					{
						strcpy(intranetip,ip);
					}
					else
					{
						strcpy(internetip,ip);
					}
					
					if (hostinfo->h_addr_list[i] + hostinfo->h_length >= hostinfo->h_name)
						break;
				}
			}
		} 
		WSACleanup();
	}
	
	returnBuf = new char[4096];
	if (strlen(internetip) == 0)
	{
		sprintf(returnBuf,server_info,name,"",intranetip,datetimebuf);
	}
	else
	{
		sprintf(returnBuf,server_info,name,internetip,intranetip,datetimebuf);
	}
	
	return returnBuf;
}
