// PrRunStatus.cpp: implementation of the PrRunStatus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrRunStatus.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrRunStatus::PrRunStatus(LogProcessor llog)
{
	log = llog;
	pr = "\0";
}

PrRunStatus::PrRunStatus()
{

}

PrRunStatus::~PrRunStatus()
{

}

BOOL PrRunStatus::Run(char* ppr,LPDEALFUNCTION dealfun)
{
	pr = ppr;
	
	DealFun = dealfun;

	if (ChkPrRunStatus(this) != -1)
		return true;
	
	return false;
}

int PrRunStatus::ChkPrRunStatus(LPVOID lParam)
{
	
	//类对象
	PrRunStatus* obj = (PrRunStatus*)lParam;
	
	//报警信息XML格式
	char xml_node[] = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"prrunstatus\">\n%s<prrunning>\n<pr string = \"%s\"/>\n</prrunning>\n</warning>--END--";

	//服务器信息
	SysInfo sys;
	char *srvInfo = sys.GetSysInfo();
	

	
	HANDLE hSnapShot;
	PROCESSENTRY32 ProcessEntry32;
	BOOL Result;
	while (1)
	{
		//获取要检测的进程数组
		if (InitMonitorPrStruct() == -1)
			return -1;

		hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		
		if (hSnapShot == INVALID_HANDLE_VALUE)
		{
			log.printf("PrRunStatus > CreateToolhelp32Snapshot() failed");
			return -1;
		}
		
		ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);
		
		Result = Process32First(hSnapShot, &ProcessEntry32);
		
		if (Result != TRUE)
		{
			CloseHandle(hSnapShot);
			log.printf("PrRunStatus > Process32First() failed");
			return -1;
		}
		
		//检测进程是否运行
		do
		{
			for(int i=0;i<=prCount;i++)
			{
				if (strstr(prRunStatus[i].prName,ProcessEntry32.szExeFile))
				{
					//log.printf("进程%s正在运行",prRunStatus[i].prName);
					prRunStatus[i].isExit = 1;
				}
			}
			
		} while (Process32Next(hSnapShot, &ProcessEntry32));

		//报警
		for(int i=0;i<=prCount;i++)
		{
			if (prRunStatus[i].isExit == 0)
			{
				char* buffer = new char[1024];
				sprintf(buffer,xml_node,srvInfo,prRunStatus[i].prName);
				obj->DealFun(buffer);
			}
			else
			{
				prRunStatus[i].isExit = 0;
			}
		}

		CloseHandle(hSnapShot);

		//隔3秒再次检测
		Sleep(3000);
	}
	
	
	//delete[] srvInfo;
	
	return 0;
}


int PrRunStatus::InitMonitorPrStruct()
{
	if (pr == "\0")
		return -1;

	//log.printf("要检测的进程%s",pr);
	prCount = 0;
	int sindex = 0;

	for (int x=0;x<ARRFCOUNT;x++)
	{
		for (int y=0;y<ARRSCOUNT;y++)
		{
			prArr[x][y] = '\0';
		}
	}


	for (int i=0;i<strlen(pr);i++)
	{
		if ( pr[i] != ',' )
		{
			prArr[prCount][sindex] = pr[i];
			sindex++;
		}
		else
		{
			prCount ++;
			sindex = 0;
		}
		
		strcpy(prRunStatus[prCount].prName,prArr[prCount]);
		prRunStatus[prCount].isExit = 0;
	}

	return 0;
}

