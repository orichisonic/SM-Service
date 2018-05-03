// WinService.h: interface for the WinService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINSERVICE_H__7D20E46C_C55A_42CA_8FA6_81CF23894E61__INCLUDED_)
#define AFX_WINSERVICE_H__7D20E46C_C55A_42CA_8FA6_81CF23894E61__INCLUDED_

#include "LogProcessor.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class WinService  
{
public:
	char* GetServiceList();
	WinService::WinService(LogProcessor hlog);
	WinService();
	virtual ~WinService();

private:
	static void replace_chr(char *sourcechr);
private:
	LogProcessor log;
};

#endif // !defined(AFX_WINSERVICE_H__7D20E46C_C55A_42CA_8FA6_81CF23894E61__INCLUDED_)
