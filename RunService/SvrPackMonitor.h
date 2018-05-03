// SvrPackMonitor.h: interface for the SvrPackMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SVRPACKMONITOR_H__F06CFF40_838D_49BD_AFA9_5923B3EA4B24__INCLUDED_)
#define AFX_SVRPACKMONITOR_H__F06CFF40_838D_49BD_AFA9_5923B3EA4B24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"
#include "SysInfo.h"

class SvrPackMonitor  
{
public:
	SvrPackMonitor();
	SvrPackMonitor(LogProcessor llog);
	virtual ~SvrPackMonitor();
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(UINT uInterval,LPDEALFUNCTION dealfun);
private:
	int Routine( LPVOID lParam );
private://传入数据
	int interval;
	LPDEALFUNCTION DealFun;
	LogProcessor log;
	

};

#endif // !defined(AFX_SVRPACKMONITOR_H__F06CFF40_838D_49BD_AFA9_5923B3EA4B24__INCLUDED_)
