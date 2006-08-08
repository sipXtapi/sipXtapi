@echo off
@call meta\repo-ver.bat > %TEMP%\repo-ver
copy meta\repo-ver.txt+%TEMP%\repo-ver  %TEMP%\setit.bat
call %TEMP%\setit.bat
del %TEMP%\setit.bat
