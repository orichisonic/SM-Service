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
	//�������
	Memory* obj = (Memory*)lParam;
	//xml������ʽ
	char xml_node[1024] = "rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"memory\">\n%s<memory>\n<rate string = \"There are %ld total Mbytes of physical memory.%ld free Mbytes of physical memory.%ld total Mbytes of virtual memory.\"/>\n</memory>\n</warning>--END--";
	//ϵͳ��Ϣ
	SysInfo sys;

	while (1)
	{
		MEMORYSTATUS memStatus;
		memStatus.dwLength=sizeof(MEMORYSTATUS);
		//���ϵͳ��ǰ������������ڴ��ʹ�����
		GlobalMemoryStatus(&memStatus);	   
		//����ܹ��������ڴ�
		DWORD tom=memStatus.dwTotalPhys;
		//��ÿ��õ������ڴ�
		DWORD mem=memStatus.dwAvailPhys;
		//��ÿ��õ������ڴ�
		DWORD res=memStatus.dwAvailVirtual;			
		//����ѷ�����ڴ�ٷֱ�
		DWORD dwAssign = (DWORD)((float)(tom - mem)/(float)tom*100);	 

		//log.printf("%ld--%ld",dwAssign,(DWORD)uiMemoryAlarmNum);
		//��������ڴ���ڱ�����
		if (dwAssign > (DWORD)uiMemoryAlarmNum)	  
		{
			//�ܹ��ڴ����ֽڴ��
			tom=(tom/1024)/1024;
			//�����ڴ����ֽڴ��
			mem=(mem/1024)/1024;
	        //���������ڴ��ֽڴ��
	    	res=(res/1024)/1024;					
			
			//����
			char* buffer = new char[1024];
			char* srvInfo = sys.GetSysInfo();
			sprintf(buffer,xml_node,srvInfo,tom,mem,res);
			delete[] srvInfo;
			//��������
			obj->DealFun(buffer);
		}

		Sleep(1000);
	}

	return 0;
}