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
		log.printf("Cpus > ��ȡNtQuerySystemInformation��ַʧ�ܣ�");
        return -1;
	}

	
    //��ȡ����������
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR)
	{
		log.printf("Cpus > ��ȡSysBaseInfo����ʧ�ܣ�");
        return -1;
	}
    

	while(1)
    {
        //��ȡϵͳʱ��
		status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status!=NO_ERROR)
		{
			log.printf("Cpus > ��ȡSysTimeInfo����ʧ�ܣ�");
            return -1;
		}
		
        //��ȡCPU����ʱ��
        status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (status != NO_ERROR)
		{
			log.printf("Cpus > ��ȡSysPerfInfo����ʧ�ܣ�");
            return -1;
		}
		
		
        //��һ��ִ�У�����
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
				//��������Ϣ
				char *srvInfo = sys.GetSysInfo();
				//����
				char* buffer = new char[1024];
				sprintf(buffer,xml_node,srvInfo,(UINT)dbIdleTime);
				delete[] srvInfo;
				//��������
				obj->DealFun(buffer);
			}
		}
		
		
        //�����ȡֵ
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

