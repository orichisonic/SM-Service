// WinPr.cpp: implementation of the WinPr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinPr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WinPr::WinPr(LogProcessor hlog)
{
	log = hlog;
}

WinPr::WinPr()
{

}

WinPr::~WinPr()
{

}

BOOL WinPr::EnablePrivilege(HANDLE hToken, LPCSTR szPrivName)
{
	TOKEN_PRIVILEGES tkp;
	//修改进程权限
	LookupPrivilegeValue( NULL,szPrivName,&tkp.Privileges[0].Luid );
	tkp.PrivilegeCount=1;
	tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	//通知系统修改进程权限
	AdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );
	
	return( (GetLastError()==ERROR_SUCCESS) );
}

CString WinPr::GetPrList()
{
	CString xml_begin = "<Process>";
	CString xml_end = "</Process>";
	LPSTR xml = "<PInfo><QuotaNonPagedPoolUsage value=\"0\"/><TerminationDate value=\"\"/><ParentProcessId value=\"0\"/><Status value=\"\"/><CSName value=\"\"/><OSName value=\"\"/><PeakPageFileUsage value=\"0\"/><PageFileUsage value=\"0\"/><QuotaPeakNonPagedPoolUsage value=\"0\"/><WorkingSetSize value=\"0\"/><CreationDate value=\"0\"/><HandleCount value=\"0\"/><PeakWorkingSetSize value=\"0\"/><WindowsVersion value=\"0\"/><MaximumWorkingSetSize value=\"\"/><PeakVirtualSize value=\"0\"/><SessionId value=\"0\"/><WriteOperationCount value=\"0\"/><PrivatePageCount value=\"0\"/><Caption value=\"\"/><ProcessId value=\"0\"/><CommandLine value=\"\"/><OSCreationClassName value=\"\"/><MinimumWorkingSetSize value=\"\"/><Priority value=\"0\"/><CreationClassName value=\"\"/><ThreadCount value=\"0\"/><KernelModeTime value=\"0\"/><ExecutionState value=\"\"/><InstallDate value=\"\"/><CSCreationClassName value=\"\"/><OtherTransferCount value=\"0\"/><WriteTransferCount value=\"0\"/><PageFaults value=\"0\"/><VirtualSize value=\"0\"/><QuotaPagedPoolUsage value=\"0\"/><ReadTransferCount value=\"0\"/><OtherOperationCount value=\"0\"/><ReadOperationCount value=\"0\"/><QuotaPeakPagedPoolUsage value=\"0\"/><ExecutablePath value=\"%s\"/><Description value=\"\"/><Name value=\"\"/><UserModeTime value=\"0\"/><Handle value=\"0\"/><PStatus value=\"0\"/></PInfo>";
	CString xml_data="";

	DWORD processid[1024],needed,processcount,i;
	HANDLE hProcess;
	HMODULE hModule;
	char path[MAX_PATH] = "",temp[256];
	HANDLE hToken;
	//char *returnbuff,*tmp;
	//bool isFirst = true;
	
	
	//给获得的当前的进程开放附加的访问令牌
	if ( OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{   
		//
		if (EnablePrivilege(hToken,SE_DEBUG_NAME))
		{
			EnumProcesses(processid, sizeof(processid), &needed);
			processcount=needed/sizeof(DWORD);
			
			for (i=0;i<processcount;i++)
			{
				hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,processid[i]);
				if (hProcess)
				{
					EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed);
					GetModuleFileNameEx(hProcess, hModule, path, sizeof(path));
					GetLongPathName(path,path,256);
					itoa(processid[i],temp,10);
					
					xml_data.Format(xml,path);
					xml_begin += xml_data;
					//tmp =  new char[strlen(xml) + strlen(path) + 1];
					//sprintf(tmp, xml,path);
					
					/*
					if (isFirst)
					{
						returnbuff = new char[strlen(tmp) + 1];//(char *)malloc(strlen(tmp));
						strcpy ( returnbuff,  tmp);	
						isFirst = false;		
					}
					else
					{
						char *tmp_rtrnbuf = new char[strlen(returnbuff) + strlen(tmp) + 1];//(char *)malloc(strlen(returnbuff) + strlen(tmp));
						strcpy(tmp_rtrnbuf,returnbuff);
						strcat(tmp_rtrnbuf,tmp);
						
						//移除内存
						delete[] returnbuff;
						
						
						returnbuff = new char[strlen(tmp_rtrnbuf) + 1];//(char *)malloc(strlen(tmp_rtrnbuf));
						strcpy(returnbuff,tmp_rtrnbuf);
						
						//移除内存
						delete[] tmp_rtrnbuf;
					}//end else*/
					//移除内存
					//delete[] tmp;
				}//end if
				CloseHandle(hProcess);
				Sleep(10);
			}//end for
		}//end if
	}//end if

	xml_begin += xml_end;
	
	CloseHandle(hToken);
	CloseHandle(hModule);	
	
	return xml_begin;
}
