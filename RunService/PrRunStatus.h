// PrRunStatus.h: interface for the PrRunStatus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRRUNSTATUS_H__09789B7F_6E9D_4ED4_A580_610F02B89927__INCLUDED_)
#define AFX_PRRUNSTATUS_H__09789B7F_6E9D_4ED4_A580_610F02B89927__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"
#include "SysInfo.h"

#define ARRFCOUNT 20	//数组第一维度
#define ARRSCOUNT 256	//数组第二维度

//进程运行状态
typedef struct
{
    char prName[ARRSCOUNT];
	int isExit;
} PR_MONITOR_STATUS;

class PrRunStatus  
{
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(char* pr,LPDEALFUNCTION dealfun);
public:
	PrRunStatus(LogProcessor llog);
	PrRunStatus();
	virtual ~PrRunStatus();

private:
	static DWORD WINAPI Routine( LPVOID lParam );
	int ChkPrRunStatus(LPVOID lParam);
	int InitMonitorPrStruct();
private:
	LPDEALFUNCTION DealFun;
	LogProcessor log;
	char* pr;
	char prArr[ARRFCOUNT][ARRSCOUNT];//处理后的进程数组
	int prCount;//实际要检测的进程个数
	PR_MONITOR_STATUS prRunStatus[ARRFCOUNT];//进程状态信息结构

};

#endif // !defined(AFX_PRRUNSTATUS_H__09789B7F_6E9D_4ED4_A580_610F02B89927__INCLUDED_)
