@echo off


%cd%\sc -stop ServerMonitor
%cd%\sc -stop XYNTService
%cd%\sc -remove ServerMonitor
%cd%\sc -remove XYNTService

cls


%cd%\sc -install ServerMonitor %cd%\RunService.exe
%cd%\sc -install XYNTService %cd%\XYNTService.exe

%cd%\sc -start ServerMonitor
%cd%\sc -start XYNTService


@echo on
@echo ...........................................................
@echo ��װ�ɹ�
@echo ...........................................................
@echo .
@echo ����ĸ�Ŀ¼�µ� RunService.ini �ļ����������£�
@echo .
@echo ��������ַ
@echo Address = ���ƻ�IP
@echo .
@echo ��ʱ�������ݴ���˿�
@echo Port = ��Server�˶�Ӧ
@echo .
@echo ʵʱ�������ݴ���˿�
@echo RPort = ��Server�˶�Ӧ
@echo .
@echo ����ʱ����  ��λ������
@echo Interval = 15
@echo .
@echo ��ش���
@echo MonitorDir = ��Ҫ������ϵͳ����,�� C:\\
@echo .
@echo .
@echo .
@echo ...........................................................
@echo �뽫 XYNTService.ini �ļ��ڵ�
@echo "CommandLine =" ��ֵ�ĳ� ��ǰ����·��\update.exe
@echo "WorkingDir = " ��ֵ�ĳ� ��ǰ����·��
@echo .
@echo ע�⣺�����װ�����б������ٴ����д˰�װ����
@echo off
pause
	