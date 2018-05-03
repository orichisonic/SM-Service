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
@echo 安装成功
@echo ...........................................................
@echo .
@echo 请更改根目录下的 RunService.ini 文件，具体如下：
@echo .
@echo 服务器地址
@echo Address = 控制机IP
@echo .
@echo 定时监听数据传输端口
@echo Port = 与Server端对应
@echo .
@echo 实时监听数据传输端口
@echo RPort = 与Server端对应
@echo .
@echo 运行时间间隔  单位：分钟
@echo Interval = 15
@echo .
@echo 监控磁盘
@echo MonitorDir = 需要监听的系统磁盘,如 C:\\
@echo .
@echo .
@echo .
@echo ...........................................................
@echo 请将 XYNTService.ini 文件内的
@echo "CommandLine =" 的值改成 当前绝对路径\update.exe
@echo "WorkingDir = " 的值改成 当前绝对路径
@echo .
@echo 注意：如果安装过程中报错，请再次运行此安装程序
@echo off
pause
	