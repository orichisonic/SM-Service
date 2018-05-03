
#include "stdafx.h"
#include <stdio.h>
#include "mydll.h"
#include "WinSock.h"
#include "ServerInfo.h"




BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


#ifdef WIN95
#pragma code_seg("_INIT")
#pragma comment(linker,"/SECTION:.bss,RWS /SECTION:.data,RWS /SECTION:.rdata,RWS /SECTION:.text,RWS /SECTION:_INIT,RWS ")
#pragma comment(linker,"/BASE:0xBFF70000")
#endif

int WriteLoAboutServer(char *fmt,...)
{
	char szTemp[4096];

	va_list marker;
	va_start(marker, fmt );     
	int ret=vsprintf(szTemp,fmt,marker);

	
	va_list args;
	char modname[200];

	char temp[5000];
	HANDLE hFile;

	GetModuleFileName(NULL, modname, sizeof(modname));

	if((hFile =CreateFile("c:\\hookapi.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) <0)
	{
		return 1;
	}
	
	_llseek((HFILE)hFile, 0, SEEK_END);

	wsprintf(temp, "mydll.dll:%s:", modname);
	DWORD dw;
	WriteFile(hFile, temp, strlen(temp), &dw, NULL);
	
	va_start(args,fmt);
	vsprintf(temp, fmt, args);
	va_end(args);

	WriteFile(hFile, temp, strlen(temp), &dw, NULL);

	wsprintf(temp, "\r\n");
	WriteFile(hFile, temp, strlen(temp), &dw, NULL);

	_lclose((HFILE)hFile);

	return 1;
}

void WriteLog(char *fmt,...)
{
	char szTemp[4096];

	va_list marker;
	va_start(marker, fmt );     
	int ret=vsprintf(szTemp,fmt,marker);

	
	
	


	//发送数据
	WinSock winsock;
	//winsock.SocketSendData("127.0.0.1",10118,szTemp,strlen(szTemp));
	winsock.SendData(dest_ip_addr,iPort,szTemp);

	//WriteLoAboutServer("---WriteLog-----Address:%s,Port:%d",serverInfo->dest_ip_addr,serverInfo->iPort);
}





char *GetRootKey(HKEY hKey)
{

	if(hKey == HKEY_CLASSES_ROOT)
		return "HKEY_CLASSES_ROOT";
	else if(hKey == HKEY_CURRENT_CONFIG)
		return "KEY_CURRENT_CONFIG";
	else if(hKey ==HKEY_CURRENT_USER)
		return "HKEY_CURRENT_USER";
	else if(hKey == HKEY_LOCAL_MACHINE)
		return "HKEY_LOCAL_MACHINE";
	else if(hKey == HKEY_USERS)
		return "HKEY_USERS";
	else if(hKey == HKEY_PERFORMANCE_DATA)
		return "HKEY_PERFORMANCE_DATA";
	else if(hKey == HKEY_DYN_DATA)
		return "HKEY_DYN_DATA";
	else
		return "未知根路径";
}

/*Delete*/
DWORD WINAPI myRegDeleteKeyA(HKEY hKey,LPCSTR lpSubKey)
{
	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	
	int ret =RegDeleteKeyA(hKey, lpSubKey);
	int err =GetLastError();
	//WriteLog("%s, myRegDeleteKeyA:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), lpSubKey);

	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;

	
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">\n%s\n<reg method=\"RegDeleteKeyA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp,GetRootKey(hKey),lpSubKey);
	
	SetLastError(err);

	return ret;
}

DWORD WINAPI myRegDeleteKeyW (HKEY hKey,LPCWSTR lpSubKey)
{
	int ret = RegDeleteKeyW(hKey, lpSubKey);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	//WriteLog("%s, myRegDeleteKeyW:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), subkey);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">\n%s\n<reg method=\"RegDeleteKeyW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp,GetRootKey(hKey),subkey);
	

	return ret;
}

DWORD WINAPI myRegDeleteValueA(HKEY hKey,LPCTSTR lpValueName)
{
	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	
	int ret =RegDeleteValueA(hKey, lpValueName);
	int err =GetLastError();
	//WriteLog("%s, myRegDeleteValueA:hKey=%s,lpValueName=%s", temp, GetRootKey(hKey), lpValueName);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;

	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">\n%s\n<reg method=\"RegDeleteValueA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<valueName string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp,GetRootKey(hKey),lpValueName);
	
	SetLastError(err);

	return ret;
}

/*
 *	注册表删除   OK->P
*/
DWORD WINAPI myRegDeleteValueW(HKEY hKey,LPCWSTR lpSubKey)
{
	int ret = RegDeleteValueW(hKey, lpSubKey);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	//WriteLog("%s, myRegDeleteValueW:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), subkey);

	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;

	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">\n%s\n<reg method=\"RegDeleteValueW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp,GetRootKey(hKey),subkey);
	
	return ret;
}

/*添加键值*/
DWORD WINAPI myRegSetValueExA (HKEY hKey,LPCSTR lpValueName,DWORD Reserved,DWORD dwType,CONST BYTE* lpData,DWORD cbData)
{
	int ret = RegSetValueExA (hKey,lpValueName,Reserved,dwType,lpData,cbData);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	//WriteLog("%s, myRegSetValueExA:hKey=%s,SubKey=%s,Reserved=%d,dwType=%d,cbData=%d", temp, GetRootKey(hKey), lpValueName,Reserved,dwType,cbData);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegSetValueExA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<Reserved string=\"%d\"/>\n<dwType string=\"%d\"/>\n<cbData string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), lpValueName,Reserved,dwType,cbData);

	return ret;
}

DWORD WINAPI myRegSetValueExW (HKEY hKey,LPCWSTR lpValueName,DWORD Reserved,DWORD dwType,CONST BYTE* lpData,DWORD cbData)
{
	int ret = RegSetValueExW(hKey, lpValueName,Reserved,dwType,lpData,cbData);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	char valueName[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpValueName, -1, valueName, sizeof(valueName),NULL,NULL); 
	valueName[len] =0;
	//WriteLog("%s, myRegSetValueExW:hKey=%s,SubKey=%s,Reserved=%d,dwType=%d,cbData=%d", temp, GetRootKey(hKey), valueName,Reserved,dwType,cbData);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegSetValueExW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<Reserved string=\"%d\"/>\n<dwType string=\"%d\"/>\n<cbData string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), valueName,Reserved,dwType,cbData);

	return ret;
}

DWORD WINAPI myRegSetValueA (HKEY hKey,LPCSTR lpSubKey,DWORD dwType,LPCSTR lpData,DWORD cbData)
{
	
	int ret = RegSetValueA(hKey, lpSubKey,dwType,lpData,cbData);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));


	//WriteLog("%s, myRegDeleteValueW:hKey=%s,SubKey=%s,dwType=%d,lpData=%s,cbData=%d", temp, GetRootKey(hKey), lpSubKey,dwType,lpData,cbData);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegSetValueA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<dwType string=\"%d\"/>\n<lpData string=\"%s\"/>\n<cbData string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), lpSubKey,dwType,lpData,cbData);

	return ret;
}

DWORD WINAPI myRegSetValueW (HKEY hKey,LPCWSTR lpSubKey,DWORD dwType,LPCWSTR lpData,DWORD cbData)
{
	
	int ret = RegSetValueW(hKey, lpSubKey,dwType,lpData,cbData);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	
	
	

	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;

	char data[200];
	int dataLen = WideCharToMultiByte(CP_ACP,0,lpData,-1,data,sizeof(data),NULL,NULL);
	data[dataLen] = 0;


	//WriteLog("%s, myRegDeleteValueW:hKey=%s,SubKey=%s,dwType=%d,lpData=%s,cbData=%d", temp, GetRootKey(hKey), subkey,dwType,data,cbData);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegSetValueW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<dwType string=\"%d\"/>\n<lpData string=\"%s\"/>\n<cbData string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), subkey,dwType,data,cbData);


	return ret;
	
}



/*创建KEY */
DWORD WINAPI myRegCreateKeyA (HKEY hKey,LPCSTR lpSubKey,PHKEY phkResult)
{	
	int ret = RegCreateKeyA(hKey, lpSubKey,phkResult);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	
	//WriteLog("%s, myRegCreateKeyA:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), lpSubKey);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegCreateKeyA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), lpSubKey);
	return ret;
}

DWORD WINAPI myRegCreateKeyW(HKEY hKey,LPCWSTR lpSubKey,PHKEY phkResult)
{	
	int ret = RegCreateKeyW(hKey, lpSubKey,phkResult);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	//WriteLog("%s, myRegCreateKeyW:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), subkey);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">\n/>\n<reg method=\"RegCreateKeyW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), subkey);

	return ret;
}

DWORD WINAPI myRegCreateKeyExA (HKEY hKey,LPCSTR lpSubKey,DWORD Reserved,LPSTR lpClass,DWORD dwOptions,REGSAM samDesired,LPSECURITY_ATTRIBUTES lpSecurityAttributes,PHKEY phkResult,LPDWORD lpdwDisposition)
{
	int ret = RegCreateKeyExA(hKey,lpSubKey,Reserved,lpClass,dwOptions,samDesired,lpSecurityAttributes,phkResult,lpdwDisposition);
	int err =GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	
	//WriteLog("%s, myRegCreateKeyExA:hKey=%s,SubKey=%s,lpClass=%s,dwOptions=%d,samDesired=%d", temp, GetRootKey(hKey), lpSubKey,lpClass,dwOptions,samDesired);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegCreateKeyExA\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<lpClass string=\"%s\"/>\n<dwOptions string=\"%d\"/>\n<samDesired string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), lpSubKey,lpClass,dwOptions,samDesired);


	return ret;
}

DWORD WINAPI myRegCreateKeyExW (HKEY hKey,LPCWSTR lpSubKey,DWORD Reserved,LPWSTR lpClass,DWORD dwOptions,REGSAM samDesired,LPSECURITY_ATTRIBUTES lpSecurityAttributes,PHKEY phkResult,LPDWORD lpdwDisposition)
{
	int ret = RegCreateKeyExW (hKey,lpSubKey,Reserved,lpClass,dwOptions,samDesired,lpSecurityAttributes,phkResult,lpdwDisposition);
	int err=GetLastError();
	SetLastError(err);

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	//WriteLog("%s, myRegCreateKeyExW:hKey=%s,SubKey=%s,lpClass=%s,dwOptions=%d,samDesired=%d", temp, GetRootKey(hKey), subkey,lpClass,dwOptions,samDesired);
	ServerInfo srvInfo;
	char *info_result = srvInfo.GetServerInfo();
	
	char result_string[200];
	sprintf(result_string,"%s",info_result);
	delete[] info_result;
	WriteLog("rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"reg\">%s<reg method=\"RegCreateKeyExW\">\n<exec_file string=\"%s\"/>\n<hKey string=\"%s\"/>\n<subKey string=\"%s\"/>\n<lpClass string=\"%s\"/>\n<dwOptions string=\"%d\"/>\n<samDesired string=\"%d\"/>\n</reg>\n</warning>--END--",result_string,temp, GetRootKey(hKey), subkey,lpClass,dwOptions,samDesired);

	return ret;
}

//---------------------------------------------------------------------------------
DWORD WINAPI myRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));

	int ret =RegOpenKeyA(hKey, lpSubKey, phkResult);
	int err =GetLastError();
	WriteLog("%s, myRegOpenKeyA:hKey=%s,SubKey=%s, hKeyResult=%x", temp, GetRootKey(hKey), lpSubKey, *phkResult);
	SetLastError(err);

	return ret;
}

DWORD WINAPI myRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	WriteLog("%s, myRegOpenKeyW:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), subkey);

	return (DWORD)RegOpenKeyW(hKey, lpSubKey, phkResult);
}

DWORD WINAPI myRegQueryValueA(HKEY hKey, LPCSTR lpSubKey, LPSTR lpValue, PLONG lpcbValue)
{
	int err;
	LONG ret =RegQueryValueA(hKey, lpSubKey, lpValue, lpcbValue);
	err =GetLastError();
	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	if(strcmpi(lpSubKey, "UseCount") ==0)
	{
		*(int *)lpValue =0;
	}
	WriteLog("%s, myRegQueryValueA:hKey=%x,SubKey=%s", temp, hKey, lpSubKey);
	SetLastError(err);

	return (DWORD)ret;
}

DWORD WINAPI myRegQueryValueW(HKEY hKey,LPCWSTR lpSubKey, LPWSTR lpValue, PLONG lpcbValue)
{
	LONG ret =RegQueryValueW(hKey, lpSubKey, lpValue, lpcbValue);
	int err =GetLastError();

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	char subkey[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, subkey, sizeof(subkey),NULL,NULL); 
	subkey[len] =0;
	WriteLog("%s, myRegQueryValueW:hKey=%s,SubKey=%s", temp, GetRootKey(hKey), subkey);
	SetLastError(err);
	
	return (DWORD)ret;
}

DWORD WINAPI myRegQueryValueExA(HKEY hKey, LPSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LONG ret =RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	int err =GetLastError();

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	WriteLog("%s, myRegQueryValueExA:hKey=%s,ValueName=%s", temp, GetRootKey(hKey), lpValueName);
	SetLastError(err);
	
	return (DWORD)ret;
}

DWORD WINAPI myRegQueryValueExW(HKEY hKey, LPWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LONG ret =RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	int err =GetLastError();

	char temp[200];
	GetModuleFileName(NULL, temp, sizeof(temp));
	char value[200];
	int len =WideCharToMultiByte( CP_ACP, 0, lpValueName, -1, value, sizeof(value),NULL,NULL); 
	value[len] =0;
	WriteLog("%s, myRegQueryValueExW:hKey=%s,ValueName=%s", temp, GetRootKey(hKey), value);
	SetLastError(err);
	
	return (DWORD)ret;
}
//---------------------------------------------------------------------------------

MYAPIINFO myapi_info[] =
{
	//{"ADVAPI32.DLL", "RegOpenKeyW", 3, "myRegOpenKeyW"},
	//{"ADVAPI32.DLL", "RegQueryValueW", 4, "myRegQueryValueW"},
	//{"ADVAPI32.DLL", "RegOpenKeyA", 3, "myRegOpenKeyA"},
	//{"ADVAPI32.DLL", "RegQueryValueA", 4, "myRegQueryValueA"},
	//{"ADVAPI32.DLL", "RegQueryValueExW", 6, "myRegQueryValueExW"},
	//{"ADVAPI32.DLL", "RegQueryValueExA", 6, "myRegQueryValueExA"},
	
	{"ADVAPI32.DLL", "RegDeleteKeyA", 2, "myRegDeleteKeyA"},
	{"ADVAPI32.DLL", "RegDeleteValueA", 2, "myRegDeleteValueA"},
	{"ADVAPI32.DLL", "RegDeleteValueW", 2, "myRegDeleteValueW"},
	{"ADVAPI32.DLL", "RegCreateKeyW",3,"myRegCreateKeyW"},
	{"ADVAPI32.DLL", "RegCreateKeyA",3,"myRegCreateKeyA"},
	{"ADVAPI32.DLL", "RegSetValueW",5,"myRegSetValueW"},
	{"ADVAPI32.DLL", "RegSetValueA",5,"myRegSetValueA"},
	{"ADVAPI32.DLL", "RegCreateKeyExA",9,"myRegCreateKeyExA"},
	{"ADVAPI32.DLL", "RegCreateKeyExW",9,"myRegCreateKeyExW"},
	{"ADVAPI32.DLL", "RegSetValueExA",6,"myRegSetValueExA"},
	{"ADVAPI32.DLL", "RegSetValueExW",6,"myRegSetValueExW"},
	
	
		
};

MYAPIINFO *GetMyAPIInfo()
{
	return &myapi_info[0];
}

void SetServerInfo(char *pdest_ip_addr,int piPort)
{
	
	//serverInfo = pServerInfo;

	sprintf(dest_ip_addr,"%s",pdest_ip_addr);
	iPort = piPort;

	WriteLoAboutServer("------SetServerInfo:Address:%s,Port:%d",dest_ip_addr,iPort);

}
