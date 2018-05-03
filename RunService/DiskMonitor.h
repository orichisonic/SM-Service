// DiskMonitor.h: interface for the DiskMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKMONITOR_H__0181CAAF_C7CE_4CC3_A65C_82775923BF30__INCLUDED_)
#define AFX_DISKMONITOR_H__0181CAAF_C7CE_4CC3_A65C_82775923BF30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum ACTION { ADDED=1, REMOVED=2, MODIFIED=3, RENAMED=4 };

class DiskMonitor  
{
public:
	DiskMonitor();
	virtual ~DiskMonitor();
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(std::string path,LPDEALFUNCTION dealfun);


private:
	void Close();
	static DWORD WINAPI Routine( LPVOID lParam );

private:
	std::string WatchedDir;
    LPDEALFUNCTION DealFun;
    HANDLE hThread;
    HANDLE hDir;
	

};

#endif // !defined(AFX_DISKMONITOR_H__0181CAAF_C7CE_4CC3_A65C_82775923BF30__INCLUDED_)
