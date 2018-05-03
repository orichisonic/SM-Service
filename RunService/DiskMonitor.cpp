// DiskMonitor.cpp: implementation of the DiskMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiskMonitor.h"
#include "SysInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
DiskMonitor::DiskMonitor():DealFun(NULL), hThread(NULL), hDir(INVALID_HANDLE_VALUE)
{

}
//��������
DiskMonitor::~DiskMonitor()
{
	Close();
}

BOOL DiskMonitor::Run(std::string path,LPDEALFUNCTION dealfun)
{
	WatchedDir = path;
	DealFun = dealfun;
	DWORD ThreadId;
	//��������,��Routine����
	hThread=CreateThread( NULL,0,Routine,this,0,&ThreadId );
    return NULL!=hThread;
}

void DiskMonitor::Close()
{
	if( NULL != hThread )
	{
		//�����߳�
		TerminateThread( hThread, 0);  
		hThread = NULL;
	}
	if( INVALID_HANDLE_VALUE != hDir )
	{
		//�رվ��
		CloseHandle( hDir );		  
		hDir = INVALID_HANDLE_VALUE;
    }
}
//�����޸�
DWORD WINAPI DiskMonitor::Routine(LPVOID lParam)
{
	
	DiskMonitor* obj = (DiskMonitor*)lParam;
	//�����ļ����
	obj->hDir = CreateFile(		  
		obj->WatchedDir.c_str(),
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
        );
	if( INVALID_HANDLE_VALUE == obj->hDir ) return false;
	
	char buf[ 2*(sizeof(FILE_NOTIFY_INFORMATION)+MAX_PATH) ];
	FILE_NOTIFY_INFORMATION* pNotify=(FILE_NOTIFY_INFORMATION *)buf;
	DWORD BytesReturned;


	//��������Ϣ
	SysInfo sys;

	//xml format
	char xml[]="rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"file\">\n%s<file  Cmd=\"%s\">\n<FileName string=\"%s\"/>\n<DestFileName string=\"%s\" />\n</file>\n</warning>--END--";
			

	while(true)	
	{
		//չʾ�ض�Ŀ¼�仯����ϸ��Ϣ
		//��ص�Ŀ¼��������Ŀ¼��ǰ���CreateFile���ɵľ���������������ǿ���FILE_LIST_DIRECTORY Ȩ��
		if( ReadDirectoryChangesW( obj->hDir,
			//FILE_NOTIFY_INFORMATION�ṹ����WINNT.hͷ�ж��壬�����˫��ģ�����ͬ��Ҳ���첽
			pNotify,
			//��ŵĻ�����ֽڳ���
			sizeof(buf),
		    //����������ΪTRUE,ͬʱ��������������Ŀ¼;�����FALSE��������ظ�Ŀ¼
 			true,					
			//�ڸ�Ŀ¼�͸���Ŀ¼�ĸı䣬�øı����������������������ɾ���ļ�
			FILE_NOTIFY_CHANGE_FILE_NAME|
			//�ڸ�Ŀ¼�͸���Ŀ¼�ĸı䣬�ı������������ɾ��һ��Ŀ¼
			FILE_NOTIFY_CHANGE_DIR_NAME|
			//�ڸ�Ŀ¼�͸���Ŀ¼�����Ըı�
			FILE_NOTIFY_CHANGE_ATTRIBUTES|
			//�ڸ�Ŀ¼�͸���Ŀ¼���ļ����ȵĸı�
			FILE_NOTIFY_CHANGE_SIZE| 
			//�ڸ�Ŀ¼�͸���Ŀ¼�����д�ĸı�
			FILE_NOTIFY_CHANGE_LAST_WRITE|
			//�ڸ�Ŀ¼�͸���Ŀ¼�����һ�η��ʵĸı�
			FILE_NOTIFY_CHANGE_LAST_ACCESS|	
			//�ڸ�Ŀ¼�͸���Ŀ¼�Ĵ����ĸı�
			FILE_NOTIFY_CHANGE_CREATION|
			//�ڸ�Ŀ¼�͸���Ŀ¼�İ�ȫ�ĸı�
			FILE_NOTIFY_CHANGE_SECURITY,
			//�첽����ʱ����ŷ��ص��ֽ�
			&BytesReturned,	
			//�����ص�ģʽ
	    	NULL,
			//����FileIOCompletionRoutine���߳�
			NULL ) )													
		{
			char tmp[MAX_PATH], str1[MAX_PATH], str2[MAX_PATH];
			//����㵽�ڴ�飬�����൱��ZeroMemory
			memset( tmp, 0, sizeof(tmp) );
			//��ANSI�뽫���ַ����ɶ��ַ�
			WideCharToMultiByte( CP_ACP,0,pNotify->FileName,pNotify->FileNameLength/2,tmp,99,NULL,NULL);
			//���str1
			strcpy( str1, tmp ); 
			//���ļ��޸������б���,��������޸Ľ��޸ĵ��ļ��Զ��ַ����
			if( 0 != pNotify->NextEntryOffset)	 
			{
				PFILE_NOTIFY_INFORMATION p = (PFILE_NOTIFY_INFORMATION)((char*)pNotify+pNotify->NextEntryOffset);
				memset( tmp, 0, sizeof(tmp) );
				WideCharToMultiByte( CP_ACP,0,p->FileName,p->FileNameLength/2,tmp,99,NULL,NULL);
				strcpy( str2, tmp );
			}
			
			char *srvInfo = sys.GetSysInfo();
			
			char *buffer = new char[1024];
			//����ö�ٵĴ����xml��ʽ���ļ���
			switch( (ACTION)pNotify->Action )  
			{
			case ADDED:
				sprintf(buffer, xml,srvInfo, "Added",str1,"");	
				break;
			case REMOVED:
				sprintf(buffer, xml,srvInfo, "Removed",str1,"");
				break;
			case MODIFIED:
				sprintf(buffer, xml,srvInfo, "Modified",str1,"");
				break;
			case RENAMED:
				sprintf(buffer, xml,srvInfo, "Rename",str1,str2);
				break;
			}
			delete[] srvInfo;
			//ִ��RunService���첽����
			obj->DealFun( buffer );	   
		}//end if
		else
		{
			break;
		}
	}//end while
    return 0;
}
