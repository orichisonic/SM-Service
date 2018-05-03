// Memory.h: interface for the Memory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORY_H__23633244_A8C0_4017_B351_2C1D68099795__INCLUDED_)
#define AFX_MEMORY_H__23633244_A8C0_4017_B351_2C1D68099795__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"
#include "SysInfo.h"

class Memory  
{
public:
	Memory(LogProcessor llog);
	Memory();
	virtual ~Memory();
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(UINT memoryAlarmNum,LPDEALFUNCTION dealfun);
private:
	int Routine( LPVOID lParam );
private:
	UINT uiMemoryAlarmNum;
	LPDEALFUNCTION DealFun;
	LogProcessor log;

};

#endif // !defined(AFX_MEMORY_H__23633244_A8C0_4017_B351_2C1D68099795__INCLUDED_)
