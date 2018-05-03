// pus.h: interface for the Cpus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PUS_H__E7B70166_46B7_4991_B75A_BDCC94578223__INCLUDED_)
#define AFX_PUS_H__E7B70166_46B7_4991_B75A_BDCC94578223__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;






class Cpus  
{
public:
	Cpus(LogProcessor hlog);
	Cpus();
	virtual ~Cpus();
public:
	typedef void (__stdcall *LPDEALFUNCTION)(char *buffer);
	BOOL Run(UINT UCpuAlarmRateNum,LPDEALFUNCTION dealfun);
private:
	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
	int GetCpuRateXMLNode(LPVOID lParam);
private:
	PROCNTQSI NtQuerySystemInformation;
	LPDEALFUNCTION DealFun;
	LogProcessor log;
	UINT CpuAlarmRateNum;
	//HANDLE hThread;


};

#endif // !defined(AFX_PUS_H__E7B70166_46B7_4991_B75A_BDCC94578223__INCLUDED_)
