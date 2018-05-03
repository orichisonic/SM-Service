// Memory.cpp: implementation of the Memory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Memory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Memory::Memory(LogProcessor llog)
{
	log = llog;
}

Memory::Memory()
{

}

Memory::~Memory()
{

}

BOOL Memory::Run(UINT memoryAlarmNum,LPDEALFUNCTION dealfun)
{
	DealFun = dealfun;
	uiMemoryAlarmNum = memoryAlarmNum;
	
	if (Routine(this) != -1)
		return true;
	
	return false;
}


int Memory::Routine( LPVOID lParam )
{
	//本类对象
	Memory* obj = (Memory*)lParam;
	//xml报警格式
	char xml_node[1024] = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"memory\">\n%s<memory>\n<rate string = \"There are %ld total Mbytes of physical memory.%ld free Mbytes of physical memory.%ld total Mbytes of virtual memory.\"/>\n</memory>\n</warning>--END--";
	//系统信息
	SysInfo sys;

	while (1)
	{
		MEMORYSTATUS memStatus;
		memStatus.dwLength=sizeof(MEMORYSTATUS);
		//获得系统当前的物理和虚拟内存的使用情况
		GlobalMemoryStatus(&memStatus);	   
		//获得总共的物理内存
		DWORD tom=memStatus.dwTotalPhys;
		//获得可用的物理内存
		DWORD mem=memStatus.dwAvailPhys;
		//获得可用的虚拟内存
		DWORD res=memStatus.dwAvailVirtual;			
		//获得已分配的内存百分比
		DWORD dwAssign = (DWORD)((float)(tom - mem)/(float)tom*100);	 

		//log.printf("%ld--%ld",dwAssign,(DWORD)uiMemoryAlarmNum);
		//如果可用内存大于报警线
		if (dwAssign > (DWORD)uiMemoryAlarmNum)	  
		{
			//总共内存用字节存放
			tom=(tom/1024)/1024;
			//可用内存用字节存放
			mem=(mem/1024)/1024;
	        //可用虚拟内存字节存放
	    	res=(res/1024)/1024;					
			
			//数据
			char* buffer = new char[1024];
			char* srvInfo = sys.GetSysInfo();
			sprintf(buffer,xml_node,srvInfo,tom,mem,res);
			delete[] srvInfo;
			//发送数据
			obj->DealFun(buffer);
		}

		Sleep(1000);
	}

	return 0;
}