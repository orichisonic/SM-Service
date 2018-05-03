// ServerInfo.h: interface for the ServerInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERINFO_H__1176E70A_FD5A_4362_A2B8_8A29E99980CF__INCLUDED_)
#define AFX_SERVERINFO_H__1176E70A_FD5A_4362_A2B8_8A29E99980CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ServerInfo  
{
public:
	ServerInfo();
	virtual ~ServerInfo();
public:
	char* GetServerInfo();

};

#endif // !defined(AFX_SERVERINFO_H__1176E70A_FD5A_4362_A2B8_8A29E99980CF__INCLUDED_)
