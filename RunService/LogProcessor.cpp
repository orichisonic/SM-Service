// LogProcessor.cpp: implementation of the LogProcessor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogProcessor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LogProcessor::LogProcessor()
{
	
}

LogProcessor::LogProcessor(char* path)
{
	exe_path = path;
}

LogProcessor::~LogProcessor()
{
	
}

void LogProcessor::MakeFileName(void)
{
	CTime tmNow=CTime::GetCurrentTime();
	sprintf(m_szFileName,"%s\\%s_%04d_%02d_%02d.txt",exe_path,"log",tmNow.GetYear(),tmNow.GetMonth(),tmNow.GetDay());
}

BOOL LogProcessor::printf(const char* txt,...)
{
	char szTemp[4096];
	
	va_list marker;
	va_start(marker, txt );     
	int ret=vsprintf(szTemp,txt,marker);
	
	MakeFileName();
	FILE *pF=fopen(m_szFileName,"a+");
	if (pF)
	{
		//调用静态函数CFileTime表示当前的时间和日期
		CTime tmNow=CTime::GetCurrentTime();
		fprintf(pF, "[%04d-%02d-%02d %02d:%02d:%02d]	%s\n",tmNow.GetYear(),tmNow.GetMonth(),tmNow.GetDay(),tmNow.GetHour(),tmNow.GetMinute(),tmNow.GetSecond(),szTemp);
		fclose(pF);
	}
	return (0);
}
