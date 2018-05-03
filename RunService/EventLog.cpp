// EventLog.cpp: implementation of the EventLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EventLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EventLog::EventLog(LogProcessor llog)
{
	log = llog;
}

EventLog::EventLog()
{

}

EventLog::~EventLog()
{

}

BOOL EventLog::Run(LPDEALFUNCTION dealfun)
{
	DealFun = dealfun;
	
	if (GetEventLog(this) != -1)
		return true;
	
	return false;
}


//����¼��ɹ��ʹ�����־
int EventLog::GetEventLog(LPVOID lParam)
{
	//�������
	EventLog* obj = (EventLog*)lParam;

	CString xml="";
	//���xml��ʽ
	LPSTR xml_data ="rxml__<?xml version=\"1.0\" encoding=\"gb2312\" ?>\n<warning type=\"eventlog\">\n%s<el type=\"%s\">\n<source string = \"%s\" />\n<computer name = \"%s\" />\n<EventType string = \"%d\" />\n<EventCategory string = \"%d\" />\n<EventID string =\"%d\" />\n<Time string = \"%s\" />\n<Description string=\"%s\" />\n<Data string=\"%s\" />\n</el>\n</warning>--END--";
	//�ַ�����
	char sourcename[3][32]={"Application","Security","System"};
	LPSTR source,computer,string;//,data;
	SysInfo sys;
	CString sec_desc;


	char             *p;     
	HANDLE           hEventLog;     
	char             *szEvBuf;     
	EVENTLOGRECORD   *pevlr;     
	DWORD            dwRead;     
	DWORD            dwNeeded;     
	DWORD            dwBufSize   =   4096;     
	BOOL             rc;     
	BOOL             ec;
	//��ѭ��
	while (1)
	{
		for (int i=0;i<3;i++)
		{
			hEventLog=OpenEventLog(NULL,sourcename[i]);     
			if(hEventLog==NULL)      
				return -1;
			
			szEvBuf = (char*)malloc(dwBufSize);     
			if (szEvBuf == NULL)     
				return -1; 
			
			while(TRUE)
			{
				//��һ�������ض����¼���־�������ܰ����ַ�a-z���������ȡ��־
				rc = ReadEventLog(  hEventLog, 
					//��־˳���ȡ��������ʹ��EVENTLOG_BACKWARDS_READ
					EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ, 
					// ��־��¼��ƫ���������������dwReadFlags����EVENTLOG_SEEK_READʱ����
					0, 
					//typedef struct _EVENTLOGRECORD
					//	{  DWORD Length;
				    //     DWORD Reserved; 
				    //     DWORD RecordNumber;
				    //     DWORD TimeGenerated; 
				    //     DWORD TimeWritten;  
				    //     DWORD EventID;  
				    //     WORD EventType;  
				    //     WORD NumStrings;  
				    //     WORD EventCategory;  
				    //     WORD ReservedFlags;  
				    //     DWORD ClosingRecordNumber;  
				    //     DWORD StringOffset;  
				    //     DWORD UserSidLength;  
				    //     DWORD UserSidOffset;  
				    //     DWORD DataLength;  
				    //     DWORD DataOffset;
					//		} EVENTLOGRECORD, *PEVENTLOGRECORD;
					//�����������ݽṹ������ָ����ָ�Ļ�����
					(EVENTLOGRECORD*)szEvBuf,  
				    //���ֽڴ�ŵĻ��峤�ȡ���������ᾡ���ܶ�����ʺ�������峤�ȵ�������,���������������ֽ�ȥ������������������пռ�	
					dwBufSize,
					//�����趨��Ҫ�����ٵ������� 
					&dwRead, 
					//��Ҫ�������ֽڵ���һ����ڣ�ͨ��GetLastError���ش�����
					&dwNeeded
					);
				
				if(!rc)
				{     
					//��������ɹ����ط��㣬���ɹ��Ļ������㲢����ERROR_INSUFFICIENT_BUFFER
					ec = GetLastError();     
					if (ec != ERROR_INSUFFICIENT_BUFFER)
						break;
					// �ͷ���־��¼����
					free(szEvBuf); 
					//ƫ�Ƶ���һ����ڵ�ַ
					dwBufSize = dwNeeded + 1024; 
					//������һ����־��¼����
					szEvBuf = (char*)malloc(dwBufSize); 
					//�������¼�����-1
					if(szEvBuf == NULL)	   
						return -1;
					
					continue;
				}     
				if(dwRead == 0)
					break;
				
				p = szEvBuf; 
				
				do{     
					pevlr=(EVENTLOGRECORD*)p;
					
					if (pevlr->NumStrings != 517)
					{
						//source
						p = (char*)((DWORD)pevlr+sizeof(EVENTLOGRECORD));
						source = p;
						

						//computer name
						p = (char*)(p+strlen(p)+1);
						computer = p;

						//Time		����EVENTLOGRECORD��ȡ�ṹ
                        CTime tm(pevlr->TimeWritten);
						CString eventlogtime;
						eventlogtime.Format("%04d-%02d-%02d   %02d:%02d",tm.GetYear(),tm.GetMonth(),tm.GetDay(),tm.GetHour(),tm.GetMinute());
					
						//string
						if (pevlr->NumStrings != 0)
						{
							p = (char *)(pevlr)+pevlr->StringOffset;
							switch (pevlr->EventID)
							{
								case 528:
								case 540:
									SEC_DESCRIPTION_BY_WEB_WSL websl;
									sec_desc = GetDescritpion(websl,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 538:
									SEC_DESCRIPTION_BY_WSB wsb;
									sec_desc = GetDescritpion(wsb,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 551:
									SEC_DESCRIPTION_BY_WWY wwy;
									sec_desc = GetDescritpion(wwy,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 552:
									SEC_DESCRIPTION_BY_WWE wwe;
									sec_desc = GetDescritpion(wwe,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 576:
									SEC_DESCRIPTION_BY_WQL wql;
									sec_desc = GetDescritpion(wql,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 680:
									SEC_DESCRIPTION_BY_LBL lbl;
									sec_desc = GetDescritpion(lbl,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 682:
									SEC_DESCRIPTION_BY_LBE lbe;
									sec_desc = GetDescritpion(lbe,p);
									string = sec_desc.GetBuffer(0);
									break;
								case 683:
									SEC_DESCRIPTION_BY_LBS lbs;
									sec_desc = GetDescritpion(lbs,p);
									string = sec_desc.GetBuffer(0);
									break;
								default:
									p = (char *)(pevlr)+pevlr->StringOffset;
									string = p;
									break;
							}
							
						}

						//Data
						/*
						if (pevlr->DataLength != 0)
						{
							p = (char *)(pevlr)+pevlr->DataOffset;
							printf("Data:%s(",p);
							for (int i=0;i<pevlr->DataLength;i++)
							{
								printf("%x ",*(p+i));
							}
							printf(")");
							
						}*/
						char* srvInfo = sys.GetSysInfo();
						xml.Format(xml_data,srvInfo,sourcename[i],source,computer,pevlr->EventType,pevlr->EventCategory,pevlr->EventID,eventlogtime,pevlr->NumStrings != 0 ? string : "","");
						delete[] srvInfo;
						LPTSTR pXml = xml.GetBuffer(0);

						//���ͺ�����Ҫɾ��ָ�룬���Ը�Ϊ��
						char *heapXml = new char[strlen(pXml)+1];
						strcpy(heapXml,pXml);

						//����
						obj->DealFun(heapXml);
					}

					dwRead -= pevlr->Length;     
					p = (char*)((DWORD)pevlr+pevlr->Length); 
					
					Sleep(300);

				}while(dwRead>0);
				
			}
			free(szEvBuf);  
			ClearEventLog(hEventLog,NULL);
			CloseEventLog(hEventLog); 
			Sleep(1000);
		} 
		Sleep(60000);
	}
	return 0;
}


template<class T>
CString EventLog::GetDescritpion(T &sec_description_ponitor,char *pointer)
{
	CString desc = "",tmp;
	for (int i=0;i<sizeof(sec_description_ponitor)/sizeof(LPSTR);i++)
	{
		if (i != 0)
			pointer = (char *)(pointer+strlen(pointer)+1);
		
		//�ṹ��������ֵ��ָ���ַд��ṹ��						
		*((DWORD*)(&sec_description_ponitor) + i) = (DWORD)pointer;
		
		tmp.Format("%s",pointer);
		
		if (i == sizeof(sec_description_ponitor)/sizeof(LPSTR) -1)
			desc += tmp;
		else
			desc += tmp + "|";
	}
	
	return desc;
}


