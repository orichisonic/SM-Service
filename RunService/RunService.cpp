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
// ��������
//////////////////////////////////////////////////////////////////////////
LPSTR SERVICE_NAME = "NineYouServerMonitor"; 

////////////////////////////////////////////////////////////////////////// 
// ��������
//////////////////////////////////////////////////////////////////////////

//ϵͳ�������
SERVICE_STATUS	ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
//����������Ϣ
PROCESS_INFORMATION pProcInfo;

//socket
SOCKET sock;

//��ǰִ��·��
const int pathSize = 500;
//ִ��·��
char execPath[pathSize+1];
//�����ļ�
char pInitFile[pathSize+1];
//Զ�̷�������ַ
char dest_ip_addr[128];
//�������˿�
UINT dest_port;
//Զ�̷�������Ϣ��ȡ����
char exefile[256];
//����IP
char local_ip_addr[128];
//���ض˿�
UINT local_port;
//��ʱɨ��ʱ��
UINT scanInterval;
//ɨ�����
char monitorDir[128];

//CPUռ���ʾ�������
UINT CpuAlarmRateNum;
//�ڴ�ʹ��������
UINT uiMemoryAlarmNum;
//����ʹ��������
UINT uiDiskAlarmNum;
//Server Pack Patch Monitor Interval
UINT uSPPatchInterval;

//��������״̬���
char monitorPr[1024];
////////////////////////////////////////////////////////////////
//�����
////////////////////////////////////////////////////////////////

//��־��¼
LogProcessor log;

//���̼��
DiskMonitor disk;


//socket
Comm comm;

//Process
WinPr pr;

//Service
WinService srvic;


//ϵͳ��Ϣ
SysInfo sys;


//CPU
Cpus cpu;

//��������״̬
PrRunStatus prRunStatus;

//�ڴ�
Memory mem;

//���̿ռ�
DriverSpace driverspace;

//�¼���־
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
// ��������
//////////////////////////////////////////////////////////////////////////

void ServiceMain(DWORD argc, LPTSTR *argv);
void ControlHandler(DWORD request);

//��ʼ��
void InitPFL();
//��ȡ�����ļ���Ϣ
void InitIniValue();
//��ʼ��socket����
void InitSocket();

//����ʹ���ʼ��
static void MonitorDriverSpace();
//���̼��
static void MonitorDriver();
//CPUռ���ʼ��
static void MonitorCPU();
//�ڴ�ʹ���ʼ��
static void MonitorMem();
//��������״̬���
static void MonitorPrRunning();
//ע�����
static void MonitorHR();
//��ȡ�¼��鿴������
static void MonitorEventLog();
//���ϵͳserver pack patch
static void MonitorSpPatch();
//�������̣����ڶ�ȡԶ����Ϣ
BOOL StartProcess();
//�������Ƿ���
static void WorkerProc();
//��������
static void EndProcess();
//
char* strunion(char *dest,char *source,BOOL isDeleteSrc=TRUE);







//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{

	InitPFL();
	//SERVICE_TABLE_ENTRY�ṹʹ����StartServiceCtrlDispatcher�����������������еĵ��ý��̵ķ���
	SERVICE_TABLE_ENTRY ServiceTable[2];
	//һ���ս����������ڷ�����̵�ָ���ַ���
	ServiceTable[0].lpServiceName = SERVICE_NAME;
	//ServiceMain������ָ��
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
    //��ʼ���иú���
	StartServiceCtrlDispatcher(ServiceTable);


	return 0;
}


//////////////////////////////////////////////////////////////////////////int argc, char** argv
//ServiceMain�Ƿ�������
void ServiceMain(DWORD argc, LPTSTR *argv) 
{ 
	DWORD status = 0;
	DWORD specificError = 0xfffffff;
	//����������Ϊ (SERVICE_WIN32_OWN_PROCESS | SERVICE_WIN32_SHARE_PROCESS)
   	ServiceStatus.dwServiceType = SERVICE_WIN32;
	//��������ǰ״̬Ϊ����
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING; 
	//����������SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE֪ͨ
	ServiceStatus.dwControlsAccepted   =  SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	//����Ĵ��뱻Ӧ���ڷ�����������������ڿ�ʼ���߽���
	ServiceStatus.dwWin32ExitCode = 0; 
	//��������������ֹͣ������ָ����������÷��񷵻ء������ֵֻ�е�dwWin32ExitCode��Ա������ΪERROR_SERVICE_SPECIFIC_ERROR�ź���
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	//���ֵ���������������Ե��������������̵ĳ��ȿ�ʼ����������ͣ���߼����������ٸ����ӣ�����Ӧ�õ�������ʱ����ɵ�ʱ�����ӵ�ÿ��ֵ�� 
	ServiceStatus.dwCheckPoint = 0; 
	//��Ҫδ���Ŀ�ʼ������������������ʱ�䣬�ú����ʾ��
	ServiceStatus.dwWaitHint = 0; 
	//�������RegisterServiceCtrlHandler����ע���ƻ�����������
	hStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME, 
		(LPHANDLER_FUNCTION)ControlHandler
		); 
	
	//���״̬ΪSERVICE_STATUS_HANDLE
	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{ 
		long nError = GetLastError();
		//��ӡ�������
		log.printf("RegisterServiceCtrlHandler failed, error code = %d\n", nError);
        return; 
	}  
	
	//��ʼ������
	//status = GetLastError();
	if (status != NO_ERROR)
	{
		log.printf("�����ʼ��ʧ��");
		//���÷�������ǰ״̬
		ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
		//���÷����������
        ServiceStatus.dwCheckPoint         = 0; 
		
        ServiceStatus.dwWaitHint           = 0; 
        ServiceStatus.dwWin32ExitCode      = status; 
        ServiceStatus.dwServiceSpecificExitCode = specificError; 
		//��ǰ�����״̬��Ϣ�ṹ��������������ظ�RegisterServiceCtrlHandlerEx����
        SetServiceStatus(hStatus, &ServiceStatus); 
        return; 
	} 
	//����SCM������״̬
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

		
		//Զ�̷�������Ϣ����  ThreadProc��WorkerProc
		StartProcess();
		hThread[0] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)WorkerProc,NULL,0,&ThreadId[0]);

		//ʵʱ���ϵͳ����	ThreadProc��MonitorDiver
		hThread[1] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorDriver,NULL,0,&ThreadId[1]);
		
		//ʵʱ���CPU ThreadProc��MonitorCPU
		hThread[2] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorCPU,NULL,0,&ThreadId[2]);

		//ʵʱ��ؽ�������״̬ ThreadProc��MonitorPrRunning
		hThread[3] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorPrRunning,NULL,0,&ThreadId[3]);

		//ʵʱ����ڴ�ʹ���� ThreadProc��MonitorMem
		hThread[4] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorMem,NULL,0,&ThreadId[4]);
		
		//����ʹ���ʼ�� ThreadProc��MonitorDiverSpace
		hThread[5] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorDriverSpace,NULL,0,&ThreadId[5]);

		//�¼��鿴����� ThreadProc��MonitorEventLog
		hThread[6] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorEventLog,NULL,0,&ThreadId[6]);
		
		//system server pack patch monitor
		hThread[7] = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE)MonitorSpPatch,NULL,0,&ThreadId[7]);
		

		
		//////////////////////////////////////////////////////////////////////////
		//���ע���dllע��
		//////////////////////////////////////////////////////////////////////////
		
		OSVERSIONINFO   VersionInformation       ;   
		VersionInformation.dwOSVersionInfoSize   =sizeof(OSVERSIONINFO);   
        ::GetVersionEx(&VersionInformation); 
		if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			//VersionInformation.dwMinorVersion == 2 ��WIN2003
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
		
		//��������Ϣ
		char *pServerInfo = sys.GetSysInfo();
		tmp = strunion(xml_begin,pServerInfo);
		
		
		//����
		//char *pPrList = pr.GetPrList();
		//tmp = strunion(tmp,pPrList);
		
		CString prlist = pr.GetPrList();
		LPTSTR pPrList = prlist.GetBuffer(0);
		tmp = strunion(tmp,pPrList,FALSE);
		
		
		//����
		char *pService = srvic.GetServiceList();
		tmp = strunion(tmp,pService);   
		
		//��������
		comm.SendData(dest_ip_addr,dest_port,strunion(tmp,xml_end));

		//��ʱɨ��
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

			
			log.printf("����ֹͣ");

			if (UnhookAllProcess)
			{
				//ж��hook dll
				UnhookAllProcess();
				//�ͷŶ�̬���ӿ�
				FreeLibrary(hLib);
				log.printf("ж��hook dll �ɹ�");
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
				//ж��hook dll
				UnhookAllProcess();
				FreeLibrary(hLib);
				log.printf("ж��hook dll �ɹ�");
			}
			EndProcess();
			break;
    } 
	
    //���õ�ǰ״̬
    //SetServiceStatus (hStatus, &ServiceStatus); 
	if (!SetServiceStatus(hStatus,  &ServiceStatus)) 
	{ 
		long nError = GetLastError();
		log.printf("SetServiceStatus failed, error code = %d\n", nError);
    } 

}


//////////////////////////////////////////////////////////////////////////


//��ȡ�����ļ���Ϣ
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
	//GetModuleFileName�����ͷ�ȫ����
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
		log.printf("SOCKET ���Ӵ���ʧ��");
}

//////////////////////////////////////////////////////////////////////////
//�ص�����--��������
void WINAPI MonitorCallBack(char *buff)
{
	comm.SendData(dest_ip_addr,dest_port,buff);
}

//server pack patch monitor
static void MonitorSpPatch()
{
	sppatch.Run(uSPPatchInterval,MonitorCallBack);
}

//���̼��
static void MonitorDriver()
{
	disk.Run(monitorDir,MonitorCallBack);

	while(1)
	{ 
		Sleep(1000);
	}
}

//CPU���
static void MonitorCPU()
{
	cpu.Run(CpuAlarmRateNum,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}

//��������״̬���
static void MonitorPrRunning()
{
	prRunStatus.Run(monitorPr,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}


//�ڴ�ʹ���ʼ��
static void MonitorMem()
{
	mem.Run(uiMemoryAlarmNum,MonitorCallBack);
	while(1)
	{ 
		Sleep(1000);
	}
}


//����ʹ���ʼ��
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
//���ע�������ע��
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
		log.printf("��ȡ����HookAllProcess��ַʧ��");
		FreeLibrary(hLib);
		return;
	}
	
	if(UnhookAllProcess ==NULL)
	{
		log.printf("��ȡ����UnhookAllProcess��ַʧ��");
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
//�ַ�������
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
//��������
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

//�������Ƿ���
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

//��������
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


