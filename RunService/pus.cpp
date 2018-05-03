// pus.cpp: implementation of the Cpus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pus.h"
#include "SysInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Cpus::Cpus(LogProcessor hlog)
{
	log = hlog;
}

Cpus::Cpus()
{
	
}

Cpus::~Cpus()
{
}

int Cpus::GetCpuRateXMLNode(LPVOID lParam)
{
	Cpus* obj = (Cpus*)lParam;
	//
	char xml_node[1024] = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"cpu\">\n%s<CPU>\n<rate string = \"%d\"/>\n</CPU></warning>--END--";
	SysInfo sys;

	
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime;
    double                         dbSystemTime;
    LONG                           status;
    LARGE_INTEGER                  liOldIdleTime = {0,0};
    LARGE_INTEGER                  liOldSystemTime = {0,0};
	
	
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle("ntdll"),
		"NtQuerySystemInformation"
		);
	
    if (!NtQuerySystemInformation)
	{
		log.printf("Cpus > 获取NtQuerySystemInformation地址失败！");
        return -1;
	}

	
    //获取处理器数量
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR)
	{
		log.printf("Cpus > 获取SysBaseInfo数据失败！");
        return -1;
	}
    

	while(1)
    {
        //获取系统时间
		status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status!=NO_ERROR)
		{
			log.printf("Cpus > 获取SysTimeInfo数据失败！");
            return -1;
		}
		
        //获取CPU空闲时间
        status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (status != NO_ERROR)
		{
			log.printf("Cpus > 获取SysPerfInfo数据失败！");
            return -1;
		}
		
		
        //第一次执行，跳过
		if (liOldIdleTime.QuadPart != 0)
		{
            // CurrentValue = NewValue - OldValue
            dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
            dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
			
            // CurrentCpuIdle = IdleTime / SystemTime
            dbIdleTime = dbIdleTime / dbSystemTime;
			
            // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
            dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
			
			if (CpuAlarmRateNum < (UINT)dbIdleTime)
			{
				//服务器信息
				char *srvInfo = sys.GetSysInfo();
				//数据
				char* buffer = new char[1024];
				sprintf(buffer,xml_node,srvInfo,(UINT)dbIdleTime);
				delete[] srvInfo;
				//发送数据
				obj->DealFun(buffer);
			}
		}
		
		
        //保存获取值
        liOldIdleTime = SysPerfInfo.liIdleTime;
        liOldSystemTime = SysTimeInfo.liKeSystemTime;
		
        Sleep(1000);
    }

	return 0;  
}


BOOL Cpus::Run(UINT UCpuAlarmRateNum,LPDEALFUNCTION dealfun)
{
	DealFun = dealfun;
	CpuAlarmRateNum = UCpuAlarmRateNum;

	if (GetCpuRateXMLNode(this) != -1)
		return true;

	return false;
}

