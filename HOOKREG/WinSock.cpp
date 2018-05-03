// WinSock.cpp: implementation of the WinSock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinSock.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WinSock::WinSock()
{

}

WinSock::~WinSock()
{

}

SOCKET WinSock::CreateConnectSocket(char *dest_ip_addr, int port)
{

	WSADATA Data;
	SOCKADDR_IN destSockAddr;
	SOCKET destSocket;
	unsigned long destAddr;
	int status;
	
	/* 初始化socket */
	status=WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != NO_ERROR)
	{
		//log.printf("WSAStartup 失败");
		return NULL;
	}
	
	/* 转换ip到inet_addr */
	destAddr=inet_addr(dest_ip_addr);
	/* 复制destAddr到sockaddr_in*/
	memcpy(&destSockAddr.sin_addr,&destAddr, sizeof(destAddr));
	/*指定端口*/
	destSockAddr.sin_port=htons(port);
	/*地址族*/
	destSockAddr.sin_family=AF_INET;
	
	/*创建socket */
	destSocket=socket(AF_INET, SOCK_STREAM, 0);
	if (destSocket == INVALID_SOCKET)
	{
		status=WSACleanup();
		if (status == SOCKET_ERROR)
			return NULL;
		return NULL;
	}
	
	
	/* 连接服务器*/
	status=connect(destSocket,(LPSOCKADDR) &destSockAddr,sizeof(destSockAddr));
	if (status == SOCKET_ERROR)
	{
		status=closesocket(destSocket);
		if (status == SOCKET_ERROR)
			return NULL;
		status=WSACleanup();
		if (status == SOCKET_ERROR)
			return NULL;
		
		return NULL;
	}
	
	return destSocket;
}

void WinSock::SendData(SOCKET remote_socket,char *buff)
{
	if (send(remote_socket,buff,strlen(buff),0) == SOCKET_ERROR)
	{
		//log.printf("数据发送失败，尝试再次发送");
		SendData(remote_socket,buff);
	}
	//delete[] buff;
	closesocket(remote_socket);
}

void WinSock::SendData(char *dest_ip_addr, int port,char *buff)
{
	SOCKET sock;
	if ((sock = CreateConnectSocket(dest_ip_addr,port))== NULL)
	{
		//log.printf("SOCKET 连接创建失败");
		return;
	}
	
	if (send(sock,buff,strlen(buff),0) == SOCKET_ERROR)
	{
		//log.printf("数据发送失败，尝试再次发送");
		SendData(sock,buff);
	}
	//delete[] buff;
	closesocket(sock);
}