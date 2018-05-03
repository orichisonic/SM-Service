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
	
	/* ��ʼ��socket */
	status=WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != NO_ERROR)
	{
		//log.printf("WSAStartup ʧ��");
		return NULL;
	}
	
	/* ת��ip��inet_addr */
	destAddr=inet_addr(dest_ip_addr);
	/* ����destAddr��sockaddr_in*/
	memcpy(&destSockAddr.sin_addr,&destAddr, sizeof(destAddr));
	/*ָ���˿�*/
	destSockAddr.sin_port=htons(port);
	/*��ַ��*/
	destSockAddr.sin_family=AF_INET;
	
	/*����socket */
	destSocket=socket(AF_INET, SOCK_STREAM, 0);
	if (destSocket == INVALID_SOCKET)
	{
		status=WSACleanup();
		if (status == SOCKET_ERROR)
			return NULL;
		return NULL;
	}
	
	
	/* ���ӷ�����*/
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
		//log.printf("���ݷ���ʧ�ܣ������ٴη���");
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
		//log.printf("SOCKET ���Ӵ���ʧ��");
		return;
	}
	
	if (send(sock,buff,strlen(buff),0) == SOCKET_ERROR)
	{
		//log.printf("���ݷ���ʧ�ܣ������ٴη���");
		SendData(sock,buff);
	}
	//delete[] buff;
	closesocket(sock);
}