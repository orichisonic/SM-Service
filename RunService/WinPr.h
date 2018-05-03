// WinPr.h: interface for the WinPr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINPR_H__9864369F_DC31_48C5_A35F_D8E7C82DBF49__INCLUDED_)
#define AFX_WINPR_H__9864369F_DC31_48C5_A35F_D8E7C82DBF49__INCLUDED_

#include "LogProcessor.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class WinPr  
{
public:
	CString GetPrList();
	WinPr::WinPr(LogProcessor hlog);
	WinPr();
	virtual ~WinPr();
private:
	static BOOL EnablePrivilege(HANDLE hToken,LPCSTR szPrivName);
private:
	LogProcessor log;
};

#endif // !defined(AFX_WINPR_H__9864369F_DC31_48C5_A35F_D8E7C82DBF49__INCLUDED_)
