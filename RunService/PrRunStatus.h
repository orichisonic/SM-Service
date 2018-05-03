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

#define ARRFCOUNT 20	//�����һά��
#define ARRSCOUNT 256	//����ڶ�ά��

//��������״̬
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
	char prArr[ARRFCOUNT][ARRSCOUNT];//�����Ľ�������
	int prCount;//ʵ��Ҫ���Ľ��̸���
	PR_MONITOR_STATUS prRunStatus[ARRFCOUNT];//����״̬��Ϣ�ṹ

};

#endif // !defined(AFX_PRRUNSTATUS_H__09789B7F_6E9D_4ED4_A580_610F02B89927__INCLUDED_)
