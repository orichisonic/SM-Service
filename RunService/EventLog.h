// EventLog.h: interface for the EventLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTLOG_H__83CE1BD9_FFA3_444B_AB43_0DA3045C6C61__INCLUDED_)
#define AFX_EVENTLOG_H__83CE1BD9_FFA3_444B_AB43_0DA3045C6C61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogProcessor.h"
#include "SysInfo.h"



//528 & 540
typedef struct
{
	LPSTR User_Name;//:	Administrator
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x2D2925)
	LPSTR Logon_Type;//:	7
	LPSTR Logon_Process;//:	User32  
	LPSTR Authentication_Package;//:	Negotiate
	LPSTR Workstation_Name;//:	DC-WIN2003
	LPSTR Logon_GUID;//:	-
	LPSTR Caller_User_Name;//:	DC-WIN2003$
	LPSTR Caller_Domain;//:	WORKGROUP
	LPSTR Caller_Logon_ID;//:	(0x0,0x3E7)
	LPSTR Caller_Process_ID;//: 2004
	LPSTR Transited_Services;//: -
	LPSTR Source_Network_Address;//:	192.168.24.168
	LPSTR Source_Port;//:	54634
}SEC_DESCRIPTION_BY_WEB_WSL;

//538 
typedef struct  
{
	LPSTR User_Name;//:	IUSR_DC-WIN2003
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x883A8)
	LPSTR Logon_Type;//:	8
}SEC_DESCRIPTION_BY_WSB;

//551
typedef struct  
{
	LPSTR User_Name;//:	IUSR_DC-WIN2003
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x883A8)
}SEC_DESCRIPTION_BY_WWY;

//552
typedef struct  
{
	LPSTR Domain;//:		NT AUTHORITY
	LPSTR Logon_ID;//:		(0x0,0x3E4)
	LPSTR Logon_GUID;//:	-
	
	LPSTR Target_User_Name;//:	IUSR_DC-WIN2003
	LPSTR Target_Domain;//:	DC-WIN2003
	LPSTR Target_Logon_GUID;//: -
	
	LPSTR Target_Server_Name;//:	localhost
	LPSTR Target_Server_Info;//:	localhost
	LPSTR Caller_Process_ID;//:	3360
	LPSTR Source_Network_Address;//:	-
	LPSTR Source_Port;//:	-
}SEC_DESCRIPTION_BY_WWE;


//576
typedef struct  
{
	LPSTR User_Name;//:	Administrator
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x2D2925)
	LPSTR Privileges;//:
}SEC_DESCRIPTION_BY_WQL;

//680
typedef struct 
{
	LPSTR Logon_attempt_by;//:	MICROSOFT_AUTHENTICATION_PACKAGE_V1_0
	LPSTR Logon_account;//:	IUSR_DC-WIN2003
	LPSTR Source_Workstation;//:	DC-WIN2003
	LPSTR Error_Code;//:	0x0
}SEC_DESCRIPTION_BY_LBL;

//682
typedef struct  
{
	LPSTR User_Name;//:	Administrator
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x1C12D)
	LPSTR Session_Name;//:	RDP-Tcp#2
	LPSTR Client_Name;//:	Unknown
	LPSTR Client_Address;//:	192.168.24.168
}SEC_DESCRIPTION_BY_LBE;

//683
typedef struct  
{			
	LPSTR User_Name;//:	Administrator
	LPSTR Domain;//:		DC-WIN2003
	LPSTR Logon_ID;//:		(0x0,0x1C12D)
	LPSTR Session_Name;//:	RDP-Tcp#1
	LPSTR Client_Name;//:	Unknown
	LPSTR Client_Address;//:	192.168.24.168
}SEC_DESCRIPTION_BY_LBS;

class EventLog  
{
public:
	EventLog(LogProcessor llog);
	EventLog();
	virtual ~EventLog();
public:
	typedef void (WINAPI *LPDEALFUNCTION)(LPSTR buffer);
	BOOL Run(LPDEALFUNCTION dealfun);
private:
	int GetEventLog(LPVOID lParam);
	template<class T>
	CString GetDescritpion(T &sec_description_ponitor,char *pointer);
private:
	LPDEALFUNCTION DealFun;
	LogProcessor log;

};

#endif // !defined(AFX_EVENTLOG_H__83CE1BD9_FFA3_444B_AB43_0DA3045C6C61__INCLUDED_)
