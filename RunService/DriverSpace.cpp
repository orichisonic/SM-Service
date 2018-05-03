// DriverSpace.cpp: implementation of the DriverSpace class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DriverSpace.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DriverSpace::DriverSpace(LogProcessor llog)
{
	log = llog;
}

DriverSpace::DriverSpace()
{

}

DriverSpace::~DriverSpace()
{

}


BOOL DriverSpace::Run(UINT diskAlarmNum,LPDEALFUNCTION dealfun)
{
	DealFun = dealfun;
	uiDiskAlarmNum = diskAlarmNum;
	
	if (Routine(this) != -1)
		return true;
	
	return false;
}

int DriverSpace::Routine( LPVOID lParam )
{

	//本类对象
	DriverSpace* obj = (DriverSpace*)lParam;
	//xml报警格式
	char xml_node[1024] = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"space\">\n%s<space>\n<description string = \"Disk:%s,Free:%dM,Total:%dM,Used percent:%d,Free percent:%d\n\"/>\n</space>\n</warning>--END--";
	//系统信息
	SysInfo sys;
	//无限循环
	while (1)	  
	{
		_ULARGE_INTEGER result_freespace,result_totalspace,result_g;
		CString strdir;
		int free_space,total_space;
		int free_percent;
		int used_percent;
		
		for(char cc='A';cc<='Z';cc++)
		{
			strdir.Format("%c:",cc);
			if(GetDriveType((LPCTSTR)strdir)==DRIVE_FIXED)
			{
				GetDiskFreeSpaceEx(strdir,&result_freespace,&result_totalspace,&result_g);
				//按字节存放剩余空间
				free_space = result_freespace.QuadPart/1024/1024;
				//按字节存放总共空间
				total_space = result_totalspace.QuadPart/1024/1024;	
				//根据总空间和剩余空间相减获得使用空间百分比
				used_percent = (float)(total_space-free_space)/total_space*100;	
				//根据使用空间百分比获得剩余空间百分比
				free_percent = 100-used_percent;					 
				//如果使用空间超过报警线
				if (used_percent > uiDiskAlarmNum)	  
				{
					char* srvInfo = sys.GetSysInfo(); 
					//数据
					char* buffer = new char[1024];
					sprintf(buffer,xml_node,srvInfo,strdir,free_space,total_space,used_percent,free_percent);
					delete[] srvInfo;
					//发送数据
					obj->DealFun(buffer);
				}
			}
		}
		
		Sleep(5000);
	}
	
	return 0;
}
