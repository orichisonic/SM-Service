// LogProcessor.h: interface for the LogProcessor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGPROCESSOR_H__556985A6_F8FA_45DD_832E_1071B4B2F17F__INCLUDED_)
#define AFX_LOGPROCESSOR_H__556985A6_F8FA_45DD_832E_1071B4B2F17F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LogProcessor  
{
public:
	LogProcessor();
	LogProcessor(char* path);
	virtual ~LogProcessor();
public:
	//д��־
	BOOL printf(const char* txt,...);
private:
	//������־�ļ�
	void MakeFileName(void);
	
private:
	//��־�ļ���
	char m_szFileName[512];
	//����ִ��·��
	char* exe_path;
	
};

#endif // !defined(AFX_LOGPROCESSOR_H__556985A6_F8FA_45DD_832E_1071B4B2F17F__INCLUDED_)
