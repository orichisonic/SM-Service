// RunService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "omm.h"
#include "LogProcessor.h"
#include "DiskMonitor.h"
#include "SysInfo.h"
#include "WinPr.h"
#include "WinService.h"
#include "pus.h"
#include "PrRunStatus.h"
#include "Memory.h"
#include "DriverSpace.h"
#include "EventLog.h"
#include "SvrPackMonitor.h"

//////////////////////////////////////////////////////////////////////////
// 常量定义
//////////////////////////////////////////////////////////////////////////
LPSTR SERVICE_NAME = "NineYouServerMonitor"; 

////////////////////////////////////////////////////////////////////////// 
// 变量定义
//////////////////////////////////////////////////////////////////////////

//系统服务参数
SERVICE_STATUS	ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
//创建进程信息
PROCESS_INFORMATION pProcInfo;

//socket
SOCKET sock;

//当前执行路径
const int pathSize = 500;
//执行路径
char execPath[pathSize+1];
//配置文件
char pInitFile[pathSize+1];
//远程服务器地址
char dest_ip_addr[128];
//服务器端口
UINT dest_port;
//远程服务器信息获取程序
char exefile[256];
//本地IP
char local_ip_addr[128];
//本地端口
UINT local_port;
//定时扫描时间
UINT scanInterval;
//扫描磁盘
char monitorDir[128];

//CPU占用率警报底线
UINT CpuAlarmRateNum;
//内存使用率下限
UINT uiMemoryAlarmNum;
//磁盘使用率下限
UINT uiDiskAlarmNum;
//Server Pack Patch Monitor Interval
UINT uSPPatchInterval;

//进程运行状态监控
char monitorPr[1024];
////////////////////////////////////////////////////////////////
//类对象
////////////////////////////////////////////////////////////////

//日志记录
LogProcessor log;

//磁盘监控
DiskMonitor disk;


//socket
Comm comm;

//Process
WinPr pr;

//Service
WinService srvic;


//系统信息
SysInfo sys;


//CPU
Cpus cpu;

//进程运行状态
PrRunStatus prRunStatus;

//内存
Memory mem;

//磁盘空间
DriverSpace driverspace;

//事件日志
EventLog eventlog;

//
SvrPackMonitor sppatch;

//////////////////////////////////////////////////////////////////////////
// lib function
//////////////////////////////////////////////////////////////////////////

typedef int (WINAPI *FuncHookOneProcess2)(HWND hwndNotify, char *exe_name);
typedef int (WINAPI *FuncUnhookOneProcess2)(char *exe_name);
typedef int (WINAPI *FuncHookAllProcess)();
typedef int (WINAPI *FuncUnhookAllProcess)();

FuncHookOneProcess2 HookOneProcess2 = NULL;
FuncUnhookOneProcess2 UnhookOneProcess2 = NULL;
FuncHookAllProcess HookAllProcess = NULL;
FuncUnhookAllProcess UnhookAllProcess = NULL;

HINSTANCE hLib;



//////////////////////////////////////////////////////////////////////////
// 方法定义
//////////////////////////////////////////////////////////////////////////

void ServiceMain(DWORD argc, LPTSTR *argv);
void ControlHandler(DWORD request);

//初始化
void InitPFL();
//获取配置文件信息
void InitIniValue();
//初始化socket连接
void InitSocket();

//磁盘使用率监控
static void MonitorDriverSpace();
//磁盘监控
static void MonitorDriver();
//CPU占用率监控
static void MonitorCPU();
//内存使用率监控
static void MonitorMem();
//进程运行状态监控
static void MonitorPrRunning();
//注册表监控
static void MonitorHR();
//读取事件查看器内容
static void MonitorEventLog();
//监控系统server pack patch
static void MonitorSpPatch();
//创建进程，用于读取远程信息
BOOL StartProcess();
//检测进程是否开启
static void WorkerProc();
//结束进程
static void EndProcess();
//
char* strunion(char *dest,char *source,BOOL isDeleteSrc=TRUE);







//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

	InitPFL();
	//SERVICE_TABLE_ENTRY结构使用于StartServiceCtrlDispatcher函数用于设置能运行的调用进程的服务
	SERVICE_TABLE_ENTRY ServiceTable[2];
	//一个空结束的运行在服务进程的指定字符串
	ServiceTable[0].lpServiceName = SERVICE_NAME;
	//ServiceMain函数的指针
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
    //开始运行该函数
	StartServiceCtrlDispatcher(ServiceTable);


	return 0;
}


//////////////////////////////////////////////////////////////////////////int argc, char** argv
//ServiceMain是服务的入口
void ServiceMain(DWORD argc, LPTSTR *argv) 
{ 
	DWORD status = 0;
	DWORD specificError = 0xfffffff;
	//服务器类型为 (SERVICE_WIN32_OWN_PROCESS | SERVICE_WIN32_SHARE_PROCESS)
   	ServiceStatus.dwServiceType = SERVICE_WIN32;
	//服务器当前状态为开着
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING; 
	//服务器接受SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE通知
	ServiceStatus.dwControlsAccepted   =  SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	//错误的代码被应用于服务用来报告错误发生在开始或者结束
	ServiceStatus.dwWin32ExitCode = 0; 
	//当服务启动或者停止服务器指定错误代码让服务返回。这个数值只有当dwWin32ExitCode成员被设置为ERROR_SERVICE_SPECIFIC_ERROR才忽略
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	//检测值用来检测服务周期性的增长报告它进程的长度开始，结束，暂停或者继续操作。举个例子，服务应该当它启动时候到完成的时候增加的每个值。 
	ServiceStatus.dwCheckPoint = 0; 
	//需要未决的开始，结束，继续操作的时间，用毫秒表示。
	ServiceStatus.dwWaitHint = 0; 
	//服务调用RegisterServiceCtrlHandler函数注册破获服务控制请求
	hStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME, 
		(LPHANDLER_FUNCTION)ControlHandler
		); 
	
	//如果状态为SERVICE_STATUS_HANDLE
	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{ 
		long nError = GetLastError();
		//打印错误代码
		log.printf("RegisterServiceCtrlHandler failed, error code = %d\n", nError);
        return; 
	}  
	
	//初始化服务
	//status = GetLastError();
	if (status != NO_ERROR)
	{
		log.printf("服务初始化失败");
		//设置服务器当前状态
		ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
		//设置服务器切入点
        ServiceStatus.dwCheckPoint         = 0; 
		
        ServiceStatus.dwWaitHint           = 0; 
        ServiceStatus.dwWin32ExitCode      = status; 
        ServiceStatus.dwServiceSpecificExitCode = specificError; 
		//当前服务的状态信息结构句柄，这个句柄返回给RegisterServiceCtrlHandlerEx函数
        SetServiceStatus(hStatus, &ServiceStatus); 
        return; 
	} 
	//设置SCM的运行状态
	ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
    ServiceStatus.dwCheckPoint = 0; 
    ServiceStatus.dwWaitHint = 0; 

	if(!SetServiceStatus(hStatus, &ServiceStatus)) 
    { 
		long nError = GetLastError();
		log.printf("SetServiceStatus failed, error code = %d\n", nError);
    } 
	
	
	if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		DWORD ThreadId[8];
		HANDLE hThread[8];

		
		//远程服务器信息访问  ThreadProc是WorkerProc
		StartProcess();
		hThread[0] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)WorkerProc,NULL,0,&ThreadId[0]);

		//实时监控系统磁盘	ThreadProc是MonitorDiver
		hThread[1] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorDriver,NULL,0,&ThreadId[1]);
		
		//实时监控CPU ThreadProc是MonitorCPU
		hThread[2] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorCPU,NULL,0,&ThreadId[2]);

		//实时监控进程运行状态 ThreadProc是MonitorPrRunning
		hThread[3] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorPrRunning,NULL,0,&ThreadId[3]);

		//实时监控内存使用率 ThreadProc是MonitorMem
		hThread[4] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorMem,NULL,0,&ThreadId[4]);
		
		//磁盘使用率监控 ThreadProc是MonitorDiverSpace
		hThread[5] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorDriverSpace,NULL,0,&ThreadId[5]);

		//事件查看器监控 ThreadProc是MonitorEventLog
		hThread[6] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorEventLog,NULL,0,&ThreadId[6]);
		
		//system server pack patch monitor
		hThread[7] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorSpPatch,NULL,0,&ThreadId[7]);
		

		
		//////////////////////////////////////////////////////////////////////////
		//监控注册表及dll注入
		//////////////////////////////////////////////////////////////////////////
		
		OSVERSIONINFO   VersionInformation       ;   
		VersionInformation.dwOSVersionInfoSize   =sizeof(OSVERSIONINFO);   
        ::GetVersionEx(&VersionInformation); 
		if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			//VersionInformation.dwMinorVersion == 2 是WIN2003
			if (VersionInformation.dwMinorVersion <= 1)  
			{
				MonitorHR();
			}
		}
	}
	
	while(ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		
		char *xml_begin = new char[256];
		sprintf(xml_begin,"%s","xml___<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<info>\n");
		
		char *xml_end = new char[128];
		sprintf(xml_end,"%s","</info>--END--");			  
		char *tmp=NULL;			
		
		//服务器信息
		char *pServerInfo = sys.GetSysInfo();
		tmp = strunion(xml_begin,pServerInfo);
		
		
		//进程
		//char *pPrList = pr.GetPrList();
		//tmp = strunion(tmp,pPrList);
		
		CString prlist = pr.GetPrList();
		LPTSTR pPrList = prlist.GetBuffer(0);
		tmp = strunion(tmp,pPrList,FALSE);
		
		
		//服务
		char *pService = srvic.GetServiceList();
		tmp = strunion(tmp,pService);   
		
		//发送数据
		comm.SendData(dest_ip_addr,dest_port,strunion(tmp,xml_end));

		//定时扫描
		Sleep(scanInterval * 60 * 1000);
		//Sleep(1000);
	}
	
}



void ControlHandler(DWORD request)
{ 
	switch(request)
	{ 
		case SERVICE_CONTROL_STOP: 			
		case SERVICE_CONTROL_SHUTDOWN: 
			ServiceStatus.dwWin32ExitCode = 0; 
			ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
			ServiceStatus.dwCheckPoint    = 0; 
			ServiceStatus.dwWaitHint      = 0;

			
			log.printf("服务停止");

			if (UnhookAllProcess)
			{
				//卸载hook dll
				UnhookAllProcess();
				//释放动态链接库
				FreeLibrary(hLib);
				log.printf("卸载hook dll 成功");
			}

			EndProcess();
			//SetServiceStatus (hStatus, &ServiceStatus);
			if (!SetServiceStatus(hStatus, &ServiceStatus))
			{ 
				long nError = GetLastError();
				log.printf("SetServiceStatus failed, error code = %d\n", nError);
			}	
			
			return; 
        case SERVICE_CONTROL_PAUSE:
			ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
			break;
		case SERVICE_CONTROL_CONTINUE:
			ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default:
			if (UnhookAllProcess)
			{
				//卸载hook dll
				UnhookAllProcess();
				FreeLibrary(hLib);
				log.printf("卸载hook dll 成功");
			}
			EndProcess();
			break;
    } 
	
    //设置当前状态
    //SetServiceStatus (hStatus, &ServiceStatus); 
	if (!SetServiceStatus(hStatus,  &ServiceStatus)) 
	{ 
		long nError = GetLastError();
		log.printf("SetServiceStatus failed, error code = %d\n", nError);
    } 

}


//////////////////////////////////////////////////////////////////////////


//获取配置文件信息
void InitIniValue()
{
	DWORD result;
	result = GetPrivateProfileString("Server","Address","127.0.0.1",dest_ip_addr, 128,pInitFile);
	dest_port = GetPrivateProfileInt("Server","Port",10118,pInitFile);
	scanInterval = GetPrivateProfileInt("Server","Interval",15,pInitFile);
	result = GetPrivateProfileString("Server","MonitorDir","c:\\",monitorDir, 128,pInitFile);
	

	CpuAlarmRateNum = GetPrivateProfileInt("Server","CpuAlarmRateNum",30,pInitFile);
	uiMemoryAlarmNum  = GetPrivateProfileInt("Server","MemoryAlarmNum",50,pInitFile);
	uiDiskAlarmNum = GetPrivateProfileInt("Server","DiskAlarmNum",50,pInitFile);
	result = GetPrivateProfileString("Server","MonitorPr","RunService.exe",monitorPr, sizeof(monitorPr),pInitFile);
	uSPPatchInterval = GetPrivateProfileInt("Server","SPPatchInterval",60,pInitFile);


	result = GetPrivateProfileString("LOCAL","ExeFile","osinfo.exe",exefile, 128,pInitFile);
	result = GetPrivateProfileString("LOCAL","Address","127.0.0.1",local_ip_addr, 128,pInitFile);
	local_port = GetPrivateProfileInt("LOCAL","Port",10118,pInitFile);
	
}



void InitPFL()
{   
	//GetModuleFileName函数释放全质量
	DWORD dwSize = GetModuleFileName(NULL,execPath,pathSize);
	execPath[dwSize] = 0;
	if(dwSize>4)
	{
		for (int i=dwSize;i>0;i--)
		{
			if (execPath[i]=='\\')
			{
				execPath[i] = 0;
				sprintf(pInitFile,"%s\\RunService.ini",execPath);
				break;
			}
		}
	}
	else
	{
		sprintf(execPath,"%s","C:\\");
		sprintf(pInitFile,"%s","RunService.ini");
	}
	
	log = LogProcessor(execPath);
	comm = Comm(log);
	pr = WinPr(log);
	srvic = WinService(log);
	cpu = Cpus(log);
	prRunStatus = PrRunStatus(log);
	mem = Memory(log);
	driverspace = DriverSpace(log);
	eventlog = EventLog(log);
	sppatch = SvrPackMonitor(log);


	InitIniValue();
}

void InitSocket()
{
	if ((sock = comm.CreateConnectSocket(dest_ip_addr,dest_port))== NULL)
		log.printf("SOCKET 连接创建失败");
}

//////////////////////////////////////////////////////////////////////////
//回调函数--发送数据
void WINAPI MonitorCallBack(char *buff)
{
	comm.SendData(dest_ip_addr,dest_port,buff);
}

//server pack patch monitor
static void MonitorSpPatch()
{
	sppatch.Run(uSPPatchInterval,MonitorCallBack);
}

//磁盘监控
static void MonitorDriver()
{
	disk.Run(monitorDir,MonitorCallBack);

	while(1)
	{ 
		Sleep(1000);
	}
}

//CPU监控
static void MonitorCPU()
{
	cpu.Run(CpuAlarmRateNum,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}

//进程运行状态监控
static void MonitorPrRunning()
{
	prRunStatus.Run(monitorPr,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}


//内存使用率监控
static void MonitorMem()
{
	mem.Run(uiMemoryAlarmNum,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}


//磁盘使用率监控
static void MonitorDriverSpace()
{
	driverspace.Run(uiDiskAlarmNum,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}


static void MonitorEventLog()
{
	eventlog.Run(MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}

//////////////////////////////////////////////////////////////////////////
//监控注册表及进程注入
static void MonitorHR()
{

	WriteProfileString("HookAPI", "exe_name", "RunService.exe");

	int isNT =false;
	OSVERSIONINFO VersionInfo;
	VersionInfo.dwOSVersionInfoSize =sizeof(OSVERSIONINFO);

	if(!GetVersionEx(&VersionInfo))
	{
		return;
	}

	if(VersionInfo.dwPlatformId ==VER_PLATFORM_WIN32_NT)
	{
		isNT =true;
	}

	if(isNT)
		hLib =LoadLibrary("HookAPINT.dll");

	if(hLib ==NULL)
	{
		log.printf("LoadLibrary HookAPINT.dll failed");
		return;
	}
	
	HookOneProcess2 =(FuncHookOneProcess2)GetProcAddress(hLib, "HookOneProcess2");
	UnhookOneProcess2 =(FuncUnhookOneProcess2)GetProcAddress(hLib, "UnhookOneProcess2");
	HookAllProcess =(FuncHookAllProcess)GetProcAddress(hLib, "HookAllProcess");
	UnhookAllProcess =(FuncUnhookAllProcess)GetProcAddress(hLib, "UnhookAllProcess");
	
	if(HookAllProcess ==NULL)
	{
		log.printf("获取函数HookAllProcess地址失败");
		FreeLibrary(hLib);
		return;
	}
	
	if(UnhookAllProcess ==NULL)
	{
		log.printf("获取函数UnhookAllProcess地址失败");
		FreeLibrary(hLib);
		return;
	}
	if(HookAllProcess() <0)
	{
		log.printf("HookAllProcesses error!");
		UnhookAllProcess();
		FreeLibrary(hLib);
		return;
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
//字符串连接
char* strunion(char *dest,char *source,BOOL isDeleteSrc)
{
	int len = strlen ( dest ) + strlen ( source );
	char *tmp_buffer = new char[len + 1];
	strcpy ( tmp_buffer, dest );
	strcat ( tmp_buffer, source );

	delete[] dest;
	if (isDeleteSrc!=FALSE)
		delete[] source;
	
	return tmp_buffer;
}

//////////////////////////////////////////////////////////////////////////
//创建进程
BOOL StartProcess() 
{ 
	
	
	STARTUPINFO startUpInfo = { sizeof(STARTUPINFO),NULL,"",NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};  
	
	char pCommandLine[128];
	sprintf(pCommandLine,"%s\\%s %s %d",execPath,exefile,local_ip_addr,local_port);
	BOOL bUserInterface = FALSE;
	
	if(bUserInterface)
	{
		startUpInfo.wShowWindow = SW_SHOW;
		startUpInfo.lpDesktop = NULL;
	}
	else
	{
		startUpInfo.wShowWindow = SW_HIDE;
		startUpInfo.lpDesktop = "";
	}


	if(CreateProcess(
		NULL,
		pCommandLine,
		NULL,
		NULL,
		TRUE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		strlen(execPath)==0?NULL:execPath,
		&startUpInfo,
		&pProcInfo
		))
	{
	}
	else
	{
		long nError = GetLastError();
		log.printf("Failed to start program '%s', error code = %d\n", pCommandLine, nError); 
		return FALSE;
	}
	
	return TRUE;
}

//检测进程是否开启
static void WorkerProc()
{
	while(1)
	{
		::Sleep(1000);

		DWORD dwCode;
		if(::GetExitCodeProcess(pProcInfo.hProcess, &dwCode))
		{
			if(dwCode!=STILL_ACTIVE)
			{
				if(StartProcess())
				{
					log.printf("Restarted process");
				}
			}
		}
		else
		{
			long nError = GetLastError();
			log.printf("GetExitCodeProcess failed, error code = %d\n", nError);
		}		
	}
}

//结束进程
static void EndProcess()
{		
	if(pProcInfo.hProcess)
	{
		//PostThreadMessage(pProcInfo.dwThreadId,WM_QUIT,0,0);
		if (TerminateProcess(pProcInfo.hProcess,0))
			log.printf("end process");
	}	
}
//////////////////////////////////////////////////////////////////////////


