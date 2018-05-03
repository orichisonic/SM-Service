// omm.h: interface for the Comm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OMM_H__02D6FB4C_1403_43FA_98F2_C68B19B98F86__INCLUDED_)
#define AFX_OMM_H__02D6FB4C_1403_43FA_98F2_C68B19B98F86__INCLUDED_

#include "LogProcessor.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Comm  
{
public:
	void SendData(SOCKET remote_socket,char *buff);
	void SendData(char *dest_ip_addr, int port,char *buff);
	SOCKET CreateConnectSocket(char *dest_ip_addr,int port);
	Comm(LogProcessor plog);
	Comm();
	virtual ~Comm();

private:
	LogProcessor log;
};

#endif // !defined(AFX_OMM_H__02D6FB4C_1403_43FA_98F2_C68B19B98F86__INCLUDED_)
