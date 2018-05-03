// WinSock.h: interface for the WinSock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINSOCK_H__70314BD9_80A8_468E_8E49_90431571ED64__INCLUDED_)
#define AFX_WINSOCK_H__70314BD9_80A8_468E_8E49_90431571ED64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




class WinSock  
{
public:
	WinSock();
	//WinSock(LogProcessor hlog);
	virtual ~WinSock();
public:
	//创建客户端SOCKET
	SOCKET CreateConnectSocket(char *dest_ip_addr, int port);

	//数据发送,接收
	//返回-1表示失败
	void SendData(SOCKET remote_socket,char *buff);
	void SendData(char *dest_ip_addr, int port,char *buff);
private:
	//LogProcessor log;
	
	
};

#endif // !defined(AFX_WINSOCK_H__70314BD9_80A8_468E_8E49_90431571ED64__INCLUDED_)
