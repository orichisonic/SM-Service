// WinService.cpp: implementation of the WinService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinService.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WinService::WinService()
{

}

WinService::WinService(LogProcessor hlog)
{
	log = hlog;
}

WinService::~WinService()
{

}

void WinService::replace_chr(char *sourcechr)
{
	int chr_len = strlen(sourcechr);
	for(int i=0;i<chr_len;i++)
	{
		if (*(sourcechr + i) == '\"')
			*(sourcechr + i)=' ';
	}
}

char* WinService::GetServiceList()
{
	SC_HANDLE scm;
		
	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE))==NULL)
	{
		return NULL;
	}

	LPENUM_SERVICE_STATUS	lpServices					= NULL; 
	DWORD					nSize						= 0; 
	DWORD					n; 
	DWORD					nResumeHandle				= 0; 
 

	//注意分配足够的空间 
	lpServices = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, 64 * 1024);
		
	EnumServicesStatus(	scm,
						SERVICE_WIN32, 
						SERVICE_STATE_ALL,
						(LPENUM_SERVICE_STATUS)lpServices, 
						64 * 1024, 
						&nSize, 
						&n, 
						&nResumeHandle
					  );
		
	LPQUERY_SERVICE_CONFIG ServicesInfo = NULL;



	char *xml_begin;//= "<WinService>\n";
	xml_begin = new char[30];
	sprintf(xml_begin,"%s","<WinService>\n");


	char *xml_end;// = "</WinService>\n";
	xml_end = new char[30];
	sprintf(xml_end,"%s","</WinService>\n");


	char xml_body[512] = "<SVC>\n<ServiceName string=\"%s\"/>\n<DisplayName string=\"%s\"/>\n<CurrentState string=\"%d\"/>\n<Path string=\"%s\"/>\n<StartType string=\"%d\"/>\n</SVC>\n";			  
	//xml_body = new char[256];
	//sprintf(xml_body,"%s","<SVC>\n<ServiceName string=\"%s\"/>\n<DisplayName string=\"%s\"/>\n<CurrentState string=\"%d\"/>\n<Path string=\"%s\"/>\n<StartType string=\"%d\"/>\n</SVC>\n");

	char *tmp_buffer;
		
	//begin for
	for (int i = 0; i < n; i++)
	{ 
		char *tmp;
		char *buffer = new char[512];

		SC_HANDLE service = NULL;
		DWORD nResumeHandle = 0; 
		BOOL isSuccess = FALSE;

		service=OpenService(scm,lpServices[i].lpServiceName,SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);

		if (!service)
		{
			return NULL;
		}

		isSuccess = QueryServiceConfig( service,0,0,&nResumeHandle );	// 释放配置的服务器参数
		ServicesInfo = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, nResumeHandle);      //注意分配足够的空间
		isSuccess = QueryServiceConfig(service,ServicesInfo,nResumeHandle,&nResumeHandle);      //枚举各个服务信息
			
			
			
 
		if (isSuccess)
		{

			char *path_addr = new char[strlen(ServicesInfo->lpBinaryPathName) +1];
			sprintf(path_addr,"%s",ServicesInfo->lpBinaryPathName);

			replace_chr(path_addr);


			sprintf(
					buffer, 
					xml_body, 
					lpServices[i].lpServiceName,  //服务器名字
					lpServices[i].lpDisplayName,
					lpServices[i].ServiceStatus.dwCurrentState,//当前状态
					path_addr,
					ServicesInfo->dwStartType
				);

			delete[] path_addr;
		}
		else
		{
			sprintf(
					buffer, 
					xml_body, 
					lpServices[i].lpServiceName,
					lpServices[i].lpDisplayName,
					lpServices[i].ServiceStatus.dwCurrentState,
					""
				);
		}
		LocalFree(ServicesInfo);
		CloseServiceHandle(service);



		int len = strlen ( xml_begin ) + strlen ( buffer );
		tmp = new char[len + 1];
		strcpy ( tmp, xml_begin );
		strcat ( tmp, buffer );

		delete[] xml_begin;
		delete[] buffer;

		xml_begin = new char[len +1];
		strcpy(xml_begin,tmp);
		delete[] tmp;

		Sleep(10);
		
	}//end for

		
	int len = strlen ( xml_begin ) + strlen ( xml_end );
	tmp_buffer = new char[len + 1];
	strcpy ( tmp_buffer, xml_begin );
	strcat ( tmp_buffer, xml_end );

	delete[] xml_begin;
	delete[] xml_end;
		
	LocalFree(lpServices);	//改变特定的内存对象的长度和属性
	CloseServiceHandle(scm);			 //	服务器对象的关闭
		

	return tmp_buffer;
}
