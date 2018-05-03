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
//析构函数
DiskMonitor::~DiskMonitor()
{
	Close();
}

BOOL DiskMonitor::Run(std::string path,LPDEALFUNCTION dealfun)
{
	WatchedDir = path;
	DealFun = dealfun;
	DWORD ThreadId;
	//启动进程,跑Routine函数
	hThread=CreateThread( NULL,0,Routine,this,0,&ThreadId );
    return NULL!=hThread;
}

void DiskMonitor::Close()
{
	if( NULL != hThread )
	{
		//挂起线程
		TerminateThread( hThread, 0);  
		hThread = NULL;
	}
	if( INVALID_HANDLE_VALUE != hDir )
	{
		//关闭句柄
		CloseHandle( hDir );		  
		hDir = INVALID_HANDLE_VALUE;
    }
}
//磁盘修改
DWORD WINAPI DiskMonitor::Routine(LPVOID lParam)
{
	
	DiskMonitor* obj = (DiskMonitor*)lParam;
	//建立文件句柄
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


	//服务器信息
	SysInfo sys;

	//xml format
	char xml[]="rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"file\">\n%s<file  Cmd=\"%s\">\n<FileName string=\"%s\"/>\n<DestFileName string=\"%s\" />\n</file>\n</warning>--END--";
			

	while(true)	
	{
		//展示特定目录变化的详细信息
		//监控的目录句柄，这个目录由前面的CreateFile生成的句柄，这个句柄必须是开着FILE_LIST_DIRECTORY 权限
		if( ReadDirectoryChangesW( obj->hDir,
			//FILE_NOTIFY_INFORMATION结构，在WINNT.h头中定义，这个是双向的，即能同步也能异步
			pNotify,
			//存放的缓冲的字节长度
			sizeof(buf),
		    //如果这个参数为TRUE,同时还监控其下面的子目录;如果是FALSE，仅仅监控该目录
 			true,					
			//在该目录和该子目录的改变，该改变包括重命名，创建，或者删除文件
			FILE_NOTIFY_CHANGE_FILE_NAME|
			//在该目录和该子目录的改变，改变包括创建或者删除一个目录
			FILE_NOTIFY_CHANGE_DIR_NAME|
			//在该目录和该子目录的属性改变
			FILE_NOTIFY_CHANGE_ATTRIBUTES|
			//在该目录和该子目录的文件长度的改变
			FILE_NOTIFY_CHANGE_SIZE| 
			//在该目录和该子目录的最后写的改变
			FILE_NOTIFY_CHANGE_LAST_WRITE|
			//在该目录和该子目录的最后一次访问的改变
			FILE_NOTIFY_CHANGE_LAST_ACCESS|	
			//在该目录和该子目录的创建的改变
			FILE_NOTIFY_CHANGE_CREATION|
			//在该目录和该子目录的安全的改变
			FILE_NOTIFY_CHANGE_SECURITY,
			//异步调用时，存放返回的字节
			&BytesReturned,	
			//不以重叠模式
	    	NULL,
			//不以FileIOCompletionRoutine做线程
			NULL ) )													
		{
			char tmp[MAX_PATH], str1[MAX_PATH], str2[MAX_PATH];
			//填充零到内存块，作用相当于ZeroMemory
			memset( tmp, 0, sizeof(tmp) );
			//以ANSI码将宽字符换成短字符
			WideCharToMultiByte( CP_ACP,0,pNotify->FileName,pNotify->FileNameLength/2,tmp,99,NULL,NULL);
			//填充str1
			strcpy( str1, tmp ); 
			//在文件修改链表中遍历,如果存在修改将修改的文件以短字符存放
			if( 0 != pNotify->NextEntryOffset)	 
			{
				PFILE_NOTIFY_INFORMATION p = (PFILE_NOTIFY_INFORMATION)((char*)pNotify+pNotify->NextEntryOffset);
				memset( tmp, 0, sizeof(tmp) );
				WideCharToMultiByte( CP_ACP,0,p->FileName,p->FileNameLength/2,tmp,99,NULL,NULL);
				strcpy( str2, tmp );
			}
			
			char *srvInfo = sys.GetSysInfo();
			
			char *buffer = new char[1024];
			//根据枚举的存放在xml格式的文件中
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
			//执行RunService的异步调用
			obj->DealFun( buffer );	   
		}//end if
		else
		{
			break;
		}
	}//end while
    return 0;
}
