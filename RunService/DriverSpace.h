// DriverSpace.h: interface for the DriverSpace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVERSPACE_H__C951AF4D_2B10_4499_9D4F_0FF31B276256__INCLUDED_)
#define AFX_DRIVERSPACE_H__C951AF4D_2B10_4499_9D4F_0FF31B276256__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"
#include "SysInfo.h"

class DriverSpace  
{
public:
	DriverSpace(LogProcessor llog);
	DriverSpace();
	virtual ~DriverSpace();
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(UINT diskAlarmNum,LPDEALFUNCTION dealfun);
private:
	int Routine( LPVOID lParam );
private:
	UINT uiDiskAlarmNum;
	LPDEALFUNCTION DealFun;
	LogProcessor log;
};

#endif // !defined(AFX_DRIVERSPACE_H__C951AF4D_2B10_4499_9D4F_0FF31B276256__INCLUDED_)
