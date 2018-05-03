// NYOsInfoViewer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <WinSock2.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")


//////////////////////////////////////////////////////////////////////////
//	定义
//////////////////////////////////////////////////////////////////////////

//缓冲区大小
#define MAX_SIZE 1024*50

//SOCKET数据接收大小
#define RECEIVE_SIZE 1024

//服务器查询命令
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

//错误返回信息
#define ERROR_INVALID_CMD "Invaild cmd."

//临时文件
#define SHELL_RESULT_FILE_NAME "c:\\sm_shell_result.txt"


//////////////////////////////////////////////////////////////////////////
//	返回数据格式
//////////////////////////////////////////////////////////////////////////
LPSTR lpFormat = "_____________________________________________________________________\n     [%s]\n\n%s";

//////////////////////////////////////////////////////////////////////////
//	定义
//////////////////////////////////////////////////////////////////////////

//结束符号
LPSTR lpEnd = "--END--";

//打印信息
LPSTR lpPrint = "开始执行命令%s...\n";

//加密解密密钥
static unsigned char Decode6BitMask[5] = { 0xff, 0xfe, 0xfd, 0xec, 0xcd };

// 临界区结构对象
CRITICAL_SECTION g_cs;


//////////////////////////////////////////////////////////////////////////
//	函数定义
//////////////////////////////////////////////////////////////////////////

/*
 *	目的：独立线程处理
 *	参数：lparam->SOCKET
 *	结果：0
 */
DWORD WINAPI ProcessThread(LPVOID lparam);

/*
 *	目的:执行命令并返回数据
 *	参数:lpCmdLine->操作命令
 *	结果:返回查命令结果内容
 */
LPSTR RunCmd(LPSTR lpCmdLine);

/*
 *  目的:获取指定字符串长度
 */
int memlen(const char *str);

/*
 *	目的：数据加密
 *	参数：pszSrc->加密数据源地址 
 *		  pszDest->加密后数据保存地址
 *		  nSrcLen->数据源长度
 *		  nDestLen->目标数据长度
 *	结果：加密后数据长度
 */
int WINAPI Encode6BitBufA(char *pszSrc, char *pszDest, int nSrcLen, int nDestLen);

/*
 *	目的：数据解密
 *	参数：pszSrc->解密数据源地址 
 *		  pszDest->解密后数据保存地址
 *		  nDestLen->解密后数据长度
 *	结果：解密后数据长度
 */
int WINAPI Decode6BitBufA(char *pszSrc, char *pszDest, int nDestLen);


/*
*	目的：读取文件内容
*	参数：
*	结果：执行结果
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
		printf("运行本程序时请加入服务器IP及端口参数！");
		return -1;
	}


	// 初始化临界区
	InitializeCriticalSection(&g_cs);

	//IP
	LPSTR lpIP = argv[1];
	//端口
	LPSTR lpPort = argv[2];

	//初始化WINSOCK
	WSADATA  wsaData;  
	int  iRet=WSAStartup(MAKEWORD(2,2),&wsaData);  
	if(iRet!=NO_ERROR)  
		printf("WSAStartup 错误!\n");  

	//创建一个SOCKET
	SOCKET  m_socket;  
	m_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_socket==INVALID_SOCKET)  
	{  
		printf("创建SOCKET失败,错误号：%ld\n",WSAGetLastError());  
		WSACleanup();  
		return  0;  
	}  

	//绑定
	sockaddr_in  service;  
	service.sin_family=AF_INET;  
	service.sin_addr.s_addr=inet_addr(lpIP);  //地址
	service.sin_port=htons(atoi(lpPort));		 //端口

	if(bind(m_socket,(SOCKADDR*)&service,sizeof(service))==SOCKET_ERROR) 
	{  
		printf("绑定失败.\n");  
		closesocket(m_socket);  
		return  0;  
	}  
	else  
		printf("SOCKET绑定完成.\n");  

	//监听
	if(listen(m_socket,20)==SOCKET_ERROR)
	{
		printf("监听错误.\n"); 
		closesocket(m_socket);
		return 0;
	}
	else  
		printf("开始监听...\n");  

	printf("等待客户端连入...\n"); 

	//接收连接
	SOCKET  AcceptSocket;
	while(1)  
	{
		AcceptSocket=SOCKET_ERROR;  
		while(AcceptSocket==SOCKET_ERROR)
		{  
			AcceptSocket=accept(m_socket,NULL,NULL);
		}
		printf("客户端连入.\n");

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
			printf("线程创建失败.\n");
		CloseHandle(hThread);

	}
	return  0; 
}



/*
 *	目的：独立线程处理
 *	参数：lparam->SOCKET
 *	结果：0
 */
DWORD WINAPI ProcessThread(LPVOID lparam)
{
	//客户端连入的SOCKET通道
	SOCKET sock = (SOCKET)lparam;
	//接收到的数据量大小
	int iRecvNum;

	//接收数据并执行查询
	while(true)
	{
		//接收数据缓冲区
		LPSTR lpRecvBuff = new char[RECEIVE_SIZE];
		//加密后数据
		LPSTR lpDest = new char[MAX_SIZE];
		//查询结果
		LPSTR lpResult = new char[MAX_SIZE];
		//初始化查询结果
		for (int i=0;i<MAX_SIZE;i++)
		{
			*(lpResult + i) = '\0';
		}
		//接收数据
		iRecvNum = recv(sock,lpRecvBuff,RECEIVE_SIZE,0);

		int iCmdValue = atoi(lpRecvBuff);

		//非数字转换后会变为0，小于0或者大于所有命令的或值的数字都将被过滤，不被执行
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
				// 进入临界区
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

				// 离开临界区
				LeaveCriticalSection(&g_cs);
			}

			//netuser
			if((iCmdValue & cmd_netuser) == cmd_netuser)
			{
				// 进入临界区
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

				// 离开临界区
				LeaveCriticalSection(&g_cs);

			}

			//netlocalgroup
			if((iCmdValue & cmd_netlocalgroup) == cmd_netlocalgroup)
			{

				// 进入临界区
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

				// 离开临界区
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

				// 进入临界区
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

				// 离开临界区
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
				// 进入临界区
				EnterCriticalSection(&g_cs);

				printf(lpPrint,"autorun");
				system("reg add HKEY_CURRENT_USER\\Software\\Sysinternals\\AutoRuns /v EulaAccepted /t REG_DWORD /d 1 /f");
				LPSTR autorun = RunCmd("c\\autorunsc -a -m");
				system("reg delete HKEY_CURRENT_USER\\Software\\Sysinternals\\Autoruns /v EulaAccepted /f");
				char temp[MAX_SIZE];
				sprintf(temp,lpFormat,"Autoruns",autorun);
				strcat(lpResult,temp);
				delete[] autorun;

				// 离开临界区
				LeaveCriticalSection(&g_cs);
			}

			Sleep(1);

		}

		//删除接收数据指针
		delete[] lpRecvBuff;

		//数据加密
		int iLen = Encode6BitBufA(lpResult,lpDest,(int)strlen(lpResult),MAX_SIZE);

		//发送查询结果
		send(sock,lpDest,strlen(lpDest),0);
		send(sock,lpEnd,(int)strlen(lpEnd),0);


		//断开与客户端的连接
		//closesocket(sock);

		//删除加密结果指针
		delete[] lpDest;

		//删除查询结果指针
		delete[] lpResult;
	}
	return 0;
}





/*
 *	目的:执行命令并返回数据
 *	参数:lpCmdLine->操作命令
 *	结果:返回查命令结果内容
 */
LPSTR RunCmd(LPSTR lpCmdLine)
{
	STARTUPINFO si = { sizeof(STARTUPINFO),NULL,"",NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};
	PROCESS_INFORMATION pi;
	char ReadBuf[MAX_SIZE];//缓冲区
	DWORD ReadNum; 
	HANDLE hRead; // 管道读句柄
	HANDLE hWrite; // 管道写句柄 
	LPSTR lpResult = new char[MAX_SIZE];//返回结果

	for (int i=0;i<MAX_SIZE;i++)
	{
		*(lpResult + i) = '\0';
	}
	
	si.wShowWindow = SW_HIDE;
	si.lpDesktop = NULL;

	BOOL bRet = CreatePipe(&hRead, &hWrite, NULL, 0); // 创建匿名管道 
	
	/*if (bRet == TRUE) 
		printf("成功创建匿名管道!\n");
	else 
		printf("创建匿名管道失败,错误代码:%d\n", GetLastError());*/

	if (!bRet)
		return NULL;

	// 得到本进程的当前标准输出 
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);

	// 设置标准输出到匿名管道 
	SetStdHandle(STD_OUTPUT_HANDLE, hWrite);
	// 创建子进程
	bRet = CreateProcess(NULL, lpCmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
	// 恢复本进程的标准输出 
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp); 
	
	/*if (bRet == TRUE) // 输入信息 
		printf("成功创建子进程!\n"); 
	else
		printf("创建子进程失败,错误代码:%d\n", GetLastError());*/

	if (!bRet)
		return NULL;
	else
	{
		//关闭进程句柄
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	CloseHandle(hWrite); // 关闭写句柄

	// 读管道直至管道关闭
	while (ReadFile(hRead, ReadBuf, sizeof(ReadBuf), &ReadNum, NULL))
	{
		
			ReadBuf[ReadNum] = '\0'; 
			strcat(lpResult,ReadBuf);
			//printf("从管道[%s]读取%d字节数据\n", ReadBuf, ReadNum); 
	}
	CloseHandle(hRead);

	/*if (GetLastError() == ERROR_BROKEN_PIPE) // 输出信息
		printf("管道被子进程关闭\n"); 
	else 
		printf("读数据错误,错误代码:%d\n", GetLastError()); */

	return lpResult;
}



/*
 *  目的:获取指定字符串长度
 */
int memlen(const char *str)
{
	const char *eos = str;

	while(*eos++);

	return((int)(eos - str));
}



/*
 *	目的：数据加密
 *	参数：pszSrc->加密数据源地址 
 *		  pszDest->加密后数据保存地址
 *		  nSrcLen->数据源长度
 *		  nDestLen->目标数据长度
 *	结果：加密后数据长度
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
 *	目的：数据解密
 *	参数：pszSrc->解密数据源地址 
 *		  pszDest->解密后数据保存地址
 *		  nDestLen->解密后数据长度
 *	结果：解密后数据长度
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
*	目的：读取文件内容
*	参数：
*	结果：执行结果
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
		
		fgets(line,RECEIVE_SIZE,fp);//设每行最大字符数为1024
		strcat(lpResult,line);
	}
	fclose(fp);
	
	return lpResult;
}









