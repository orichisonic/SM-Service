// sc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include <winsvc.h>

LPENUM_SERVICE_STATUS EnumServices(SC_HANDLE, LPDWORD);
BOOL InstallService(SC_HANDLE hSCManager, LPCTSTR ServiceName, LPCTSTR ServiceExe);
BOOL RemoveService(SC_HANDLE hSCManager, LPCTSTR ServiceName);
BOOL StartService(SC_HANDLE hSCManager, LPCTSTR ServiceName);
BOOL StopService(SC_HANDLE hSCManager, LPCTSTR ServiceName);
//BOOL IsAdmin(void);
void err_show(char*);
void Usage(char*);


int main(int argc, char* argv[])
{
	SC_HANDLE hSCManager = NULL;
	int nRet = 1;

	//nRet = 0;//IsAdmin(); 
	/*if(!nRet)
	{
		printf("Must administrator privilege!\n");
	}*/

	//
	// 打开服务控制管理器
	//
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCManager == NULL)
	{
		fprintf(stderr, "OpenSCManager() failed. --err: %d\n", GetLastError());
		return -1;
	}


	//
	// 调用EnumServices列举系统中的服务
	//
	LPENUM_SERVICE_STATUS lpServices = NULL;
	DWORD dwServicesReturned =0;
	lpServices = EnumServices(hSCManager, &dwServicesReturned);

	if(lpServices == 0)
	{
		free(lpServices);
		CloseServiceHandle(hSCManager);
		return -1;
	}

	//
	// 显示服务信息
	//

	if(argc == 2)
	{
			//
			// 显示帮助信息
			//
			if(!stricmp(argv[1], "-h") || !stricmp(argv[1], "-help"))
			{
				Usage(argv[0]);
				return 0;
			}

			for(DWORD i = 0; i < dwServicesReturned; i++, lpServices++)
			{
				if(!stricmp(lpServices->lpServiceName, argv[1]))
					break;
			}

			/*if(i == dwServicesReturned)
			{
				printf("Service not found!\n");
				free(lpServices);
				CloseServiceHandle(hSCManager);
				return -1;
			}*/

			printf("[%s]\n", lpServices->lpDisplayName);
			printf("\tService Name: %s\n", lpServices->lpServiceName);
			printf("\tService Type: ");
			switch(lpServices->ServiceStatus.dwServiceType)
			{
				case SERVICE_FILE_SYSTEM_DRIVER:
					printf("File System Driver\n!");
					break;
				case SERVICE_KERNEL_DRIVER: 
					printf("Device Driver\n");
					break;
				default:
					printf("User-Mode Service\n");
				break;
			}

			printf("\tState: ");

			switch(lpServices->ServiceStatus.dwCurrentState)
			{
				case SERVICE_PAUSED: 
					printf("PAUSED\n");
					break;
				case SERVICE_RUNNING: 
					printf("RUNNING\n");
					break;
				case SERVICE_STOPPED: 
					printf("STOPPED\n");
					break;
				default: 
					printf("PENDING\n");
					break;
			}

			free(lpServices);

			CloseServiceHandle(hSCManager);
			return 0;
	}

	if(argc == 1)
	{
		
		for(DWORD i = 0; i < dwServicesReturned; i++, lpServices++)
		{
			printf("[%s]\n", lpServices->lpDisplayName);
			printf("\tService Name: %s\n", lpServices->lpServiceName);
			printf("\tService Type: ");
			switch(lpServices->ServiceStatus.dwServiceType)
			{
				case SERVICE_FILE_SYSTEM_DRIVER:
					printf("File System Driver\n!");
					break;
				case SERVICE_KERNEL_DRIVER: 
					printf("Device Driver\n");
					break;
				default:
					printf("User-Mode Service\n");
				break;
			}

			printf("\tState: ");

			switch(lpServices->ServiceStatus.dwCurrentState)
			{
				case SERVICE_PAUSED: 
					printf("PAUSED\n");
					break;
				case SERVICE_RUNNING: 
					printf("RUNNING\n");
					break;
				case SERVICE_STOPPED: 
					printf("STOPPED\n");
					break;
				default: 
					printf("PENDING\n");
					break;
			}


			SC_HANDLE service		= NULL;
			DWORD     nResumeHandle = 0; 
			BOOL isSuccess = FALSE;
			LPQUERY_SERVICE_CONFIG ServicesInfo = NULL;

			service=OpenService(hSCManager,lpServices->lpServiceName,SERVICE_ALL_ACCESS);

			isSuccess = QueryServiceConfig( service,0,0,&nResumeHandle );
			ServicesInfo = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, nResumeHandle);      //注意分配足够的空间 		
			
			isSuccess = QueryServiceConfig(service,ServicesInfo,nResumeHandle,&nResumeHandle);      //枚举各个服务信息
 
			if (isSuccess)
			{
				
				printf("\t程序路径: %s\n",ServicesInfo->lpBinaryPathName);
			}


			//printf("\n-----------------------------------\n");
		}
		//printf("%s [%s] [%s]\n", lpServices->lpServiceName,lpServices->lpDisplayName,lpServices->ServiceStatus);

		//printf("\n\t\tTotal %d Service(s).\n\n", dwServicesReturned);
		//free(lpServices);
		//CloseServiceHandle(hSCManager);
		return 0;

	}



// ------------------------
// !分析命令行参数
// ------------------------

//
// !安装服务
//
if(!stricmp(argv[1], "-install"))
{
if(argc != 4)
{
Usage(argv[0]);
return 0;
}
nRet = InstallService(hSCManager, argv[2], argv[3]);
if(!nRet)
{
printf("Install service failed.\n");
return -1;
}
}

//
// 卸载服务
//
if(!stricmp(argv[1], "-remove"))
{
if(argc != 3)
{
Usage(argv[0]);
return 0;
}
nRet = RemoveService(hSCManager, argv[2]);
if(!nRet)
{
printf("Remove service failed.\n");
return -1;
}
}

//
// 启动服务
//
if(!stricmp(argv[1], "-start"))
{
if(argc != 3)
{
Usage(argv[0]);
return 0;
}
nRet = StartService(hSCManager, argv[2]);
if(!nRet)
{
printf("Start service failed.\n");
return -1;
}
}

//
// 停止服务
//
if(!stricmp(argv[1], "-stop"))
{
if(argc != 3)
{
Usage(argv[0]);
return 0;
}
nRet = StopService(hSCManager, argv[2]);
if(!nRet)
{
printf("Stop service failed.\n");
return -1;
}
}
// -----------------------------------------------------

CloseServiceHandle(hSCManager);
return 0;
}

//
// EnumServices
// 列举系统的驱动
// ----------------------
// 参数：
// [IN] SC_HANDLE hSCManager 服务管理器句柄
// [OUT] LPDWORD lpdwServices 系统中安装的驱动的数量
// 返回值：
//成功返回ENUM_SERVICE_STATUS结构的指针,否则返回NULL
// 
LPENUM_SERVICE_STATUS EnumServices(SC_HANDLE hSCManager, LPDWORD lpdwServices)
{
DWORD cbBytesNeeded = 0;
DWORD cbBufSize = 0;
DWORD dwServicesReturned = 0;

int nRet = 0;

//
// 首次调用EnumServicesStatus确定缓冲区的大小,由cbBytesNeeded?祷?br>//
nRet = EnumServicesStatus(
hSCManager,
SERVICE_DRIVER,
SERVICE_STATE_ALL,
NULL,
0,
&cbBytesNeeded,
lpdwServices,
0);

LPENUM_SERVICE_STATUS lpServices = (LPENUM_SERVICE_STATUS) malloc(cbBytesNeeded);

cbBufSize = cbBytesNeeded;

nRet = EnumServicesStatus(
hSCManager,
SERVICE_DRIVER,
SERVICE_STATE_ALL,
lpServices,
cbBufSize,
&cbBytesNeeded,
lpdwServices,
0);
if(nRet == 0)
{
err_show("EnumServicesStatus()");
return NULL;
}

return lpServices;
}

//
// InstallService
// 安装服务
// 参数：
// [IN] SC_HANDLE hSCManager 服务管理器句柄
// [IN] LPCTSTR ServiceName 服务名称
// [IN] LPCTSTR ServiceExe 可执行文件(需全路径)
// 输出：
// 成功：返回TRUE，否则返回FALSE
//
BOOL InstallService(SC_HANDLE hSCManager, LPCTSTR ServiceName, LPCTSTR ServiceExe)
{
	SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
	if (schSCManager==0) 
	{
		return false;
	}
	else
	{
		SC_HANDLE schService = CreateService
		( 
			schSCManager,	/* SCManager database      */ 
			ServiceName,			/* name of service         */ 
			ServiceName,			/* service name to display */ 
			SERVICE_ALL_ACCESS,        /* desired access          */ 
			SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS , /* service type            */ 
			SERVICE_AUTO_START,      /* start type              */ 
			SERVICE_ERROR_NORMAL,      /* error control type      */ 
			ServiceExe,			/* service's binary        */ 
			NULL,                      /* no load ordering group  */ 
			NULL,                      /* no tag identifier       */ 
			NULL,                      /* no dependencies         */ 
			NULL,                      /* LocalSystem account     */ 
			NULL
		);                     /* no password             */ 
		if (schService==0) 
		{
			return false;
		}
		else
		{
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);
		
	}
	return true;

	/*
SC_HANDLE schService;

//
// so #$%@! ...:)
//
printf("Install %s... ", ServiceExe);
schService = CreateService( hSCManager, // SCManager database
ServiceName, // name of service
ServiceName, // name to display
SERVICE_ALL_ACCESS, // desired access
SERVICE_KERNEL_DRIVER, // service type
SERVICE_DEMAND_START, // start type
SERVICE_ERROR_NORMAL, // error control type
ServiceExe, // service's binary
NULL, // no load ordering group
NULL, // no tag identifier
NULL, // no dependencies
NULL, // LocalSystem account
NULL // no password
);
if (schService == NULL)
{
if(GetLastError() == ERROR_SERVICE_EXISTS)
{
printf("Service has already installed!\n");
}
err_show("CreateService()");
return FALSE;
}
printf("Ok!\n");

CloseServiceHandle(schService);
return TRUE;
*/
}

//
// StartService
// 启动服务
// ----------------
// 参数：
// [IN] SC_HANDLE hSCManager 服务管理器句柄
// [IN] LPCTSTR ServiceName 驱动名称
// 返回值：
// 成功返回TRUE，否则返回FALSE
//
BOOL StartService(SC_HANDLE hSCManager, LPCTSTR ServiceName)
{
SC_HANDLE schService = NULL;
int nRet = 0;

schService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS);
if(schService == NULL)
{
if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
{
printf("Service is not exist!\n");
return FALSE;
}
err_show("OpenService()");
return FALSE;
}

nRet = StartService(schService, 0, NULL);
if(!nRet)
{
if(GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
{
printf("Service is already running!\n");
return nRet;
}
err_show("StartService()");
}

CloseServiceHandle(schService);
return nRet;
}

//
// StopService
// 停止驱动
// ---------------
// 参数：
// [IN] SC_HANDLE hSCManager !服务管理器句柄
// [IN] LPCTSTR ServiceName 服务名称
// 返回值：
// 成功返回TRUE，否则返回FALSE
//
BOOL StopService(SC_HANDLE hSCManager, LPCTSTR ServiceName)
{
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ServiceStatus;
	int nRet = 0;

	schService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS);
	if(schService == NULL)
		return FALSE;

	nRet = ControlService(schService, SERVICE_CONTROL_STOP, &ServiceStatus);
	if(!nRet)
	{
		switch(GetLastError())
		{
			case ERROR_SERVICE_NOT_ACTIVE:
				printf("Service has stopped!\n");
				return nRet;

			case ERROR_INVALID_SERVICE_CONTROL:
				printf("The requested control code is not valid!\n");
				return nRet;
		}
			
		err_show("ControlService()");
	}

	CloseServiceHandle(schService);
	return nRet;
}

//
// RemoveService
// 卸载服务
// ------------
// 参数：
// [IN] SC_HANDLE hSCManager 服务管理器句柄
// [IN] LPCTSTR ServiceName 服务名称
// 返回值：
// 成功返回TRUE，否则返回FALSE
//
BOOL RemoveService(SC_HANDLE hSCManager, LPCTSTR ServiceName)
{
SC_HANDLE schService;
int nRet = 0;

schService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS);
if(schService == NULL)
	return FALSE;

nRet = DeleteService(schService);
if(!nRet)
{
err_show("DeleteService()");
}

CloseServiceHandle(schService);

return nRet;
}



void err_show(char* msg)
{
fprintf(stderr, "%s failed. --err: %d\n", msg, GetLastError());
}

void Usage(char* msg)
{
printf(" %s [[-install srv exe] │ [-remove srv] │ [-start srv] │ [-stop srv]] [srv]\n\n", msg);
printf("%s Show all service\n", msg);
printf(" %s srv Show status of srv_name\n", msg);
printf(" -install srv exe Install a service, and must full path of exe\n");
printf(" -remove srv Remove a service\n");
printf(" -start srv Start a service\n");
printf(" -stop srv Stop a service\n");
printf("eg.\n");
printf(" %s -install fw c:\\fw.sys", msg);
}
