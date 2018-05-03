// NYOsInfoViewer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <WinSock2.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")


//////////////////////////////////////////////////////////////////////////
//	����
//////////////////////////////////////////////////////////////////////////

//��������С
#define MAX_SIZE 1024*50

//SOCKET���ݽ��մ�С
#define RECEIVE_SIZE 1024

//��������ѯ����
#define cmd_hostname		0x1//system basic information
#define cmd_ipconfig		0x2//network information
#define cmd_netstat			0x4//prot information
#define cmd_port			0x8//processes and prot images
#define cmd_ps				0x10//process
#define cmd_rkdetector		0x20//kernel checking
#define cmd_hid_ps			0x40//hidden Process  catchme.exe	 -
#define cmd_netuser			0x80//account information     -
#define cmd_netlocalgroup	0x100//administrators informations     -
#define cmd_cca				0x200//checking the cloned accounts
#define cmd_drivers			0x400//sysem services informations
#define cmd_netshare		0x800//system share        -
#define cmd_psinfo			0x1000//installed hotfix
#define cmd_autorun			0x2000//autoruns

//���󷵻���Ϣ
#define ERROR_INVALID_CMD "Invaild cmd."

//��ʱ�ļ�
#define SHELL_RESULT_FILE_NAME "c:\\sm_shell_result.txt"


//////////////////////////////////////////////////////////////////////////
//	�������ݸ�ʽ
//////////////////////////////////////////////////////////////////////////
LPSTR lpFormat = "_____________________________________________________________________\n     [%s]\n\n%s";

//////////////////////////////////////////////////////////////////////////
//	����
//////////////////////////////////////////////////////////////////////////

//��������
LPSTR lpEnd = "--END--";

//��ӡ��Ϣ
LPSTR lpPrint = "��ʼִ������%s...\n";

//���ܽ�����Կ
static unsigned char Decode6BitMask[5] = { 0xff, 0xfe, 0xfd, 0xec, 0xcd };

// �ٽ����ṹ����
CRITICAL_SECTION g_cs;


//////////////////////////////////////////////////////////////////////////
//	��������
//////////////////////////////////////////////////////////////////////////

/*
 *	Ŀ�ģ������̴߳���
 *	������lparam->SOCKET
 *	�����0
 */
DWORD WINAPI ProcessThread(LPVOID lparam);

/*
 *	Ŀ��:ִ�������������
 *	����:lpCmdLine->��������
 *	���:���ز�����������
 */
LPSTR RunCmd(LPSTR lpCmdLine);

/*
 *  Ŀ��:��ȡָ���ַ�������
 */
int memlen(const char *str);

/*
 *	Ŀ�ģ����ݼ���
 *	������pszSrc->��������Դ��ַ 
 *		  pszDest->���ܺ����ݱ����ַ
 *		  nSrcLen->����Դ����
 *		  nDestLen->Ŀ�����ݳ���
 *	��������ܺ����ݳ���
 */
int WINAPI Encode6BitBufA(char *pszSrc, char *pszDest, int nSrcLen, int nDestLen);

/*
 *	Ŀ�ģ����ݽ���
 *	������pszSrc->��������Դ��ַ 
 *		  pszDest->���ܺ����ݱ����ַ
 *		  nDestLen->���ܺ����ݳ���
 *	��������ܺ����ݳ���
 */
int WINAPI Decode6BitBufA(char *pszSrc, char *pszDest, int nDestLen);


/*
*	Ŀ�ģ���ȡ�ļ�����
*	������
*	�����ִ�н��
*/
LPSTR ReadShellResultFile();






int _tmain(int argc, _TCHAR* argv[])
{
	int paranum = 0;
	while(1)
	{
		if (*(argv + paranum) != '\0')
			paranum += 1;
		else
			break;
	}

	if (paranum < 3)
	{
		printf("���б�����ʱ����������IP���˿ڲ�����");
		return -1;
	}


	// ��ʼ���ٽ���
	InitializeCriticalSection(&g_cs);

	//IP
	LPSTR lpIP = argv[1];
	//�˿�
	LPSTR lpPort = argv[2];

	//��ʼ��WINSOCK
	WSADATA  wsaData;  
	int  iRet=WSAStartup(MAKEWORD(2,2),&wsaData);  
	if(iRet!=NO_ERROR)  
		printf("WSAStartup ����!\n");  

	//����һ��SOCKET
	SOCKET  m_socket;  
	m_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_socket==INVALID_SOCKET)  
	{  
		printf("����SOCKETʧ��,����ţ�%ld\n",WSAGetLastError());  
		WSACleanup();  
		return  0;  
	}  

	//��
	sockaddr_in  service;  
	service.sin_family=AF_INET;  
	service.sin_addr.s_addr=inet_addr(lpIP);  //��ַ
	service.sin_port=htons(atoi(lpPort));		 //�˿�

	if(bind(m_socket,(SOCKADDR*)&service,sizeof(service))==SOCKET_ERROR) 
	{  
		printf("��ʧ��.\n");  
		closesocket(m_socket);  
		return  0;  
	}  
	else  
		printf("SOCKET�����.\n");  

	//����
	if(listen(m_socket,20)==SOCKET_ERROR)
	{
		printf("��������.\n"); 
		closesocket(m_socket);
		return 0;
	}
	else  
		printf("��ʼ����...\n");  

	printf("�ȴ��ͻ�������...\n"); 

	//��������
	SOCKET  AcceptSocket;
	while(1)  
	{
		AcceptSocket=SOCKET_ERROR;  
		while(AcceptSocket==SOCKET_ERROR)
		{  
			AcceptSocket=accept(m_socket,NULL,NULL);
		}
		printf("�ͻ�������.\n");

		DWORD  dwThreadId;
		HANDLE hThread;  

		hThread=CreateThread(
			NULL,
			NULL,
			ProcessThread,  
			(LPVOID)AcceptSocket,
			0,
			&dwThreadId
			);

		if(hThread==NULL)   
			printf("�̴߳���ʧ��.\n");
		CloseHandle(hThread);

	}
	return  0; 
}



/*
 *	Ŀ�ģ������̴߳���
 *	������lparam->SOCKET
 *	�����0
 */
DWORD WINAPI ProcessThread(LPVOID lparam)
{
	//�ͻ��������SOCKETͨ��
	SOCKET sock = (SOCKET)lparam;
	//���յ�����������С
	int iRecvNum;

	//�������ݲ�ִ�в�ѯ
	while(true)
	{
		//�������ݻ�����
		LPSTR lpRecvBuff = new char[RECEIVE_SIZE];
		//���ܺ�����
		LPSTR lpDest = new char[MAX_SIZE];
		//��ѯ���
		LPSTR lpResult = new char[MAX_SIZE];
		//��ʼ����ѯ���
		for (int i=0;i<MAX_SIZE;i++)
		{
			*(lpResult + i) = '\0';
		}
		//��������
		iRecvNum = recv(sock,lpRecvBuff,RECEIVE_SIZE,0);

		int iCmdValue = atoi(lpRecvBuff);

		//������ת������Ϊ0��С��0���ߴ�����������Ļ�ֵ�����ֶ��������ˣ�����ִ��
		int allcmd = cmd_hostname | cmd_ipconfig | cmd_netstat | cmd_port | cmd_ps | cmd_rkdetector | cmd_hid_ps | cmd_netuser | cmd_netlocalgroup | cmd_cca | cmd_drivers | cmd_netshare | cmd_psinfo | cmd_autorun;
		if ((iCmdValue > allcmd) || (iCmdValue <= 0))
			strcpy(lpResult,ERROR_INVALID_CMD);
		else
		{
			//hostname
			if((iCmdValue & cmd_hostname) == cmd_hostname)
			{
				printf(lpPrint,"hostname");
				LPSTR hostname = RunCmd("hostname");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"System Basic Information",hostname);
				strcat(lpResult,temp);
				delete[] hostname;
			}

			//ipconfig
			if((iCmdValue & cmd_ipconfig) == cmd_ipconfig)
			{
				printf(lpPrint,"ipconfig");
				LPSTR ipconfig = RunCmd("ipconfig /all");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Network Information",ipconfig);
				strcat(lpResult,temp);
				delete[] ipconfig;
			}

			//netstat
			if((iCmdValue & cmd_netstat) == cmd_netstat)
			{
				printf(lpPrint,"netstat");
				LPSTR netstat = RunCmd("netstat -an");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Port Information",netstat);
				strcat(lpResult,temp);
				delete[] netstat;
			}

			//netport
			if((iCmdValue & cmd_port) == cmd_port)
			{
				printf(lpPrint,"netport");
				LPSTR netport = RunCmd("c\\netport.exe");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Processes and Port Images",netport);
				strcat(lpResult,temp);
				delete[] netport;
			}

			//ps
			if((iCmdValue & cmd_ps) == cmd_ps)
			{
				printf(lpPrint,"ps");
				LPSTR ps = RunCmd("c\\ps.exe -t");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Process",ps);
				strcat(lpResult,temp);
				delete[] ps;
			}

			//rkdetector
			if((iCmdValue & cmd_rkdetector) == cmd_rkdetector)
			{
				printf(lpPrint,"rkdetector");
				LPSTR rkdetector = RunCmd("c\\rkdetector.exe");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Kernel Checking",rkdetector);
				strcat(lpResult,temp);
				delete[] rkdetector;
			}

			//hid_ps
			if((iCmdValue & cmd_hid_ps) == cmd_hid_ps)
			{
				// �����ٽ���
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"hid_ps");
				
				char cmd_content[128];
				sprintf(cmd_content,"echo. > enter",SHELL_RESULT_FILE_NAME);
				system(cmd_content);
				sprintf(cmd_content,"c\\catchme.exe < enter > %s",SHELL_RESULT_FILE_NAME);
				system(cmd_content);
				system("del enter");
				LPSTR hid_ps = ReadShellResultFile();
				
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Hidden Processes - Services - Files Checking",hid_ps);
				strcat(lpResult,temp);
				delete[] hid_ps;

				// �뿪�ٽ���
				LeaveCriticalSection(&g_cs);
			}

			//netuser
			if((iCmdValue & cmd_netuser) == cmd_netuser)
			{
				// �����ٽ���
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"netuser");

				char cmd_content[128];
				sprintf(cmd_content,"net user > %s",SHELL_RESULT_FILE_NAME);
				system(cmd_content);

				LPSTR netuser = ReadShellResultFile();								
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Account Information",netuser);
				strcat(lpResult,temp);
				delete[] netuser;

				// �뿪�ٽ���
				LeaveCriticalSection(&g_cs);

			}

			//netlocalgroup
			if((iCmdValue & cmd_netlocalgroup) == cmd_netlocalgroup)
			{

				// �����ٽ���
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"netlocalgroup");

				char cmd_content[128];
				sprintf(cmd_content,"net localgroup administrators > %s",SHELL_RESULT_FILE_NAME);
				system(cmd_content);

				LPSTR netlocalgroup = ReadShellResultFile();	
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Administrators Informations",netlocalgroup);
				strcat(lpResult,temp);
				delete[] netlocalgroup;

				// �뿪�ٽ���
				LeaveCriticalSection(&g_cs);				
			}

			//cca
			if((iCmdValue & cmd_cca) == cmd_cca)
			{
				printf(lpPrint,"cca");
				LPSTR cca = RunCmd("c\\cca.exe");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Checking the Cloned Accounts",cca);
				strcat(lpResult,temp);
				delete[] cca;
			}

			//drivers
			if((iCmdValue & cmd_drivers) == cmd_drivers)
			{
				printf(lpPrint,"drivers");
				LPSTR drivers = RunCmd("c\\cca.exe");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Sysem Services Informations",drivers);
				strcat(lpResult,temp);
				delete[] drivers;
			}

			//netshare
			if((iCmdValue & cmd_netshare) == cmd_netshare)
			{

				// �����ٽ���
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"netshare");
			
				char cmd_content[128];
				sprintf(cmd_content,"net share > %s",SHELL_RESULT_FILE_NAME);
				system(cmd_content);

				LPSTR netshare = ReadShellResultFile();	
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"System Share",netshare);
				strcat(lpResult,temp);
				delete[] netshare;

				// �뿪�ٽ���
				LeaveCriticalSection(&g_cs);
			}

			//psinfo
			if((iCmdValue & cmd_psinfo) == cmd_psinfo)
			{
				printf(lpPrint,"psinfo");
				LPSTR psinfo = RunCmd("c\\psinfo -h");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Installed HotFix",psinfo);
				strcat(lpResult,temp);
				delete[] psinfo;
			}

			//autorun
			if((iCmdValue & cmd_autorun) == cmd_autorun)
			{
				// �����ٽ���
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"autorun");
				system("reg add HKEY_CURRENT_USER\\Software\\Sysinternals\\AutoRuns /v EulaAccepted /t REG_DWORD /d 1 /f");
				LPSTR autorun = RunCmd("c\\autorunsc -a -m");
				system("reg delete HKEY_CURRENT_USER\\Software\\Sysinternals\\Autoruns /v EulaAccepted /f");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Autoruns",autorun);
				strcat(lpResult,temp);
				delete[] autorun;

				// �뿪�ٽ���
				LeaveCriticalSection(&g_cs);
			}

			Sleep(1);

		}

		//ɾ����������ָ��
		delete[] lpRecvBuff;

		//���ݼ���
		int iLen = Encode6BitBufA(lpResult,lpDest,(int)strlen(lpResult),MAX_SIZE);

		//���Ͳ�ѯ���
		send(sock,lpDest,strlen(lpDest),0);
		send(sock,lpEnd,(int)strlen(lpEnd),0);


		//�Ͽ���ͻ��˵�����
		//closesocket(sock);

		//ɾ�����ܽ��ָ��
		delete[] lpDest;

		//ɾ����ѯ���ָ��
		delete[] lpResult;
	}
	return 0;
}





/*
 *	Ŀ��:ִ�������������
 *	����:lpCmdLine->��������
 *	���:���ز�����������
 */
LPSTR RunCmd(LPSTR lpCmdLine)
{
	STARTUPINFO si = { sizeof(STARTUPINFO),NULL,"",NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};
	PROCESS_INFORMATION pi;
	char ReadBuf[MAX_SIZE];//������
	DWORD ReadNum; 
	HANDLE hRead; // �ܵ������
	HANDLE hWrite; // �ܵ�д��� 
	LPSTR lpResult = new char[MAX_SIZE];//���ؽ��

	for (int i=0;i<MAX_SIZE;i++)
	{
		*(lpResult + i) = '\0';
	}
	
	si.wShowWindow = SW_HIDE;
	si.lpDesktop = NULL;

	BOOL bRet = CreatePipe(&hRead, &hWrite, NULL, 0); // ���������ܵ� 
	
	/*if (bRet == TRUE) 
		printf("�ɹ����������ܵ�!\n");
	else 
		printf("���������ܵ�ʧ��,�������:%d\n", GetLastError());*/

	if (!bRet)
		return NULL;

	// �õ������̵ĵ�ǰ��׼��� 
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);

	// ���ñ�׼����������ܵ� 
	SetStdHandle(STD_OUTPUT_HANDLE, hWrite);
	// �����ӽ���
	bRet = CreateProcess(NULL, lpCmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
	// �ָ������̵ı�׼��� 
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp); 
	
	/*if (bRet == TRUE) // ������Ϣ 
		printf("�ɹ������ӽ���!\n"); 
	else
		printf("�����ӽ���ʧ��,�������:%d\n", GetLastError());*/

	if (!bRet)
		return NULL;
	else
	{
		//�رս��̾��
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	CloseHandle(hWrite); // �ر�д���

	// ���ܵ�ֱ���ܵ��ر�
	while (ReadFile(hRead, ReadBuf, sizeof(ReadBuf), &ReadNum, NULL))
	{
		
			ReadBuf[ReadNum] = '\0'; 
			strcat(lpResult,ReadBuf);
			//printf("�ӹܵ�[%s]��ȡ%d�ֽ�����\n", ReadBuf, ReadNum); 
	}
	CloseHandle(hRead);

	/*if (GetLastError() == ERROR_BROKEN_PIPE) // �����Ϣ
		printf("�ܵ����ӽ��̹ر�\n"); 
	else 
		printf("�����ݴ���,�������:%d\n", GetLastError()); */

	return lpResult;
}



/*
 *  Ŀ��:��ȡָ���ַ�������
 */
int memlen(const char *str)
{
	const char *eos = str;

	while(*eos++);

	return((int)(eos - str));
}



/*
 *	Ŀ�ģ����ݼ���
 *	������pszSrc->��������Դ��ַ 
 *		  pszDest->���ܺ����ݱ����ַ
 *		  nSrcLen->����Դ����
 *		  nDestLen->Ŀ�����ݳ���
 *	��������ܺ����ݳ���
 */
int WINAPI Encode6BitBufA(char *pszSrc, char *pszDest, int nSrcLen, int nDestLen)
{
	int				nDestPos	= 0;
	int				nRestCount	= 0;
	char	chMade = 0, chRest = 0;

	for (int i = 0; i < nSrcLen; i++)
	{
		if (nDestPos >= nDestLen) break;

		chMade = ((chRest | (pszSrc[i] >> (2 + nRestCount))) & 0x3f);
		chRest = (((pszSrc[i] << (8 - (2 + nRestCount))) >> 2) & 0x3f);

		nRestCount += 2;

		if (nRestCount < 6)
			pszDest[nDestPos++] = chMade + 0x3c;
		else
		{
			if (nDestPos < nDestLen - 1)
			{
				pszDest[nDestPos++]	= chMade + 0x3c;
				pszDest[nDestPos++]	= chRest + 0x3c;
			}
			else
				pszDest[nDestPos++] = chMade + 0x3c;

			nRestCount	= 0;
			chRest		= 0;
		}
	}

	if (nRestCount > 0)
		pszDest[nDestPos++] = chRest + 0x3c;

	pszDest[nDestPos] = '\0';

	return nDestPos;
}



/*
 *	Ŀ�ģ����ݽ���
 *	������pszSrc->��������Դ��ַ 
 *		  pszDest->���ܺ����ݱ����ַ
 *		  nDestLen->���ܺ����ݳ���
 *	��������ܺ����ݳ���
 */
int WINAPI Decode6BitBufA(char *pszSrc, char *pszDest, int nDestLen)
{
	int				nLen = memlen((const char *)pszSrc) - 1;
	int				nDestPos = 0, nBitPos = 2;
	int				nMadeBit = 0;
	unsigned char	ch, chCode, tmp;

	for (int i = 0; i < nLen; i++)
	{
		if ((pszSrc[i] - 0x3c) >= 0)
			ch = pszSrc[i] - 0x3c;
		else
		{
			nDestPos = 0;
			break;
		}

		if (nDestPos >= nDestLen) break;

		if ((nMadeBit + 6) >= 8)
		{
			chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
			pszDest[nDestPos++] = chCode;

			nMadeBit = 0;

			if (nBitPos < 6) 
				nBitPos += 2;
			else
			{
				nBitPos = 2;
				continue;
			}
		}

		tmp = ((ch << nBitPos) & Decode6BitMask[nBitPos - 2]);

		nMadeBit += (8 - nBitPos);
	}

	pszDest[nDestPos] = '\0';

	return nDestPos;
}

/*
*	Ŀ�ģ���ȡ�ļ�����
*	������
*	�����ִ�н��
*/
LPSTR ReadShellResultFile()
{
	FILE *fp;   
	LPSTR lpResult = new char[MAX_SIZE];

	for (int i=0;i<MAX_SIZE;i++)
	{
		*(lpResult + i) = '\0';
	}

	char line[RECEIVE_SIZE];

	fp = fopen(SHELL_RESULT_FILE_NAME,"r");   
	while(!feof(fp))   
	{   
		
		fgets(line,RECEIVE_SIZE,fp);//��ÿ������ַ���Ϊ1024
		strcat(lpResult,line);
	}
	fclose(fp);
	
	return lpResult;
}









