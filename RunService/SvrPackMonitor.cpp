// SvrPackMonitor.cpp: implementation of the SvrPackMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SvrPackMonitor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SvrPackMonitor::SvrPackMonitor()
{

}

SvrPackMonitor::SvrPackMonitor(LogProcessor llog)
{
	log = llog;
}

SvrPackMonitor::~SvrPackMonitor()
{

}

BOOL SvrPackMonitor::Run(UINT uInterval,LPDEALFUNCTION dealfun)
{
	interval = uInterval;

	DealFun = dealfun;
	
	if (Routine(this) != -1)
		return true;
	
	return false;
}

int SvrPackMonitor::Routine(LPVOID lParam )
{
	//本类对象
	SvrPackMonitor* obj = (SvrPackMonitor*)lParam;
	//xml报警格式
	LPSTR xml_head = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"ServerPack\">\n%s<SP>\n";
	LPSTR xml_body = "<HotFixID string = \"%s\"/>\n";
	CString xml_end  = "</SP>\n</warning>--END--";
	//系统信息
	SysInfo sys;
	
	
	while (1)
	{
		char patchname[256];
		HKEY hKey;
		DWORD i;
		DWORD retCode;
		DWORD dwxValueName=256;
		CString full_xml;
		CString node_xml;

		//系统信息
		char *systemInfo = sys.GetSysInfo();
		full_xml.Format(xml_head,systemInfo);
		delete[] systemInfo;

		//patch
		LPCTSTR path="SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix";
		//打开的注册表键值
		LONG return0=RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_READ,&hKey);
		if(return0!=ERROR_SUCCESS)	  
		{
			return -1;
		}
		for(i=1,retCode=ERROR_SUCCESS;retCode==ERROR_SUCCESS;i++)
		{

			retCode=RegEnumKey(hKey,i,patchname,MAX_PATH);
			if(retCode==ERROR_SUCCESS) 
			{
				node_xml.Format(xml_body,patchname);
				full_xml += node_xml;
				//printf("%s\n",patchname);

			}
		}
		::RegCloseKey(hKey);
		full_xml += xml_end;

		char *sppatchbuffer = full_xml.GetBuffer(0);
		char *sendxml = new char[strlen(sppatchbuffer)+1];
		strcpy(sendxml,sppatchbuffer);
		
		
		obj->DealFun(sendxml);

		//定时扫描
		Sleep(interval * 60 * 1000);
		log.printf("svrpackmonitor -- interval is %d",interval);
		//Sleep(1000);
	}
	
	return 0;
}
