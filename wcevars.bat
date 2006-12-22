@echo off
REM  Set environment for the emulator target
REM  This batch file serves as a template only.
Title WCE Build Environment


REM This batch file sets up an environment for building for a specific CPU from the command line.
REM The build environment defaults to the SDK for Pocket PC 2003 platform. The macros that control the
REM platform are: PLATFORM, OSVERSION, WCEROOT and SDKROOT. The STANDARDSDK_500 default settings
REM are as follows:
REM    	PLATFORM=STANDARDSDK_500
REM        OSVERSION=WCE500
REM        WCEROOT=C:\Program Files\Microsoft eMbedded C++ 4.0    //Root dir for VCCE
REM        SDKROOT=C:\Program Files\Windows CE Tools    			//Root dir for STANDARDSDK_500
REM The batch file uses these macros to set the PATH, INCLUDE, LIB macros for the default
REM platform. Please note that if the default setup options were altered during
REM installation (for example, if the install directories were changed), then the user needs
REM to modify these macros accordingly.

if "%PLATFORM%"=="" echo "Using defaults for PLATFORM"
if "%PLATFORM%"=="" echo "This probably isn't what you want - consult readme.txt"

if "%OSVERSION%"=="" set OSVERSION=WCE500
if "%PLATFORM%"=="" set PLATFORM=STANDARDSDK_500
if "%WCEROOT%"=="" set WCEROOT=C:\Program Files\Microsoft eMbedded C++ 4.0
if "%SDKROOT%"=="" set SDKROOT=C:\Program Files\Windows CE Tools

Title WCE %OSVERSION% %PLATFORM% Build Environment

set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%OSVERSION%\bin;%path%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include\emulator;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\emulator;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\x86;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\x86;

if "%CC%"=="" set CC=cl.exe

if "%TARGETCPU%"=="" set TARGETCPU=emulator
if "%CFG%"=="" set CFG=none
echo.
echo  Setup to use eVC is complete.
echo.
echo  To compile STLport, cd to your stlport directory,
echo  then cd to the build\lib directory underneath it:
echo.
echo  cd STLport\build\lib
echo.
echo  Then, to build STLPort do the following:
echo.
echo    configure -c evc4 --rtl-dynamic
echo    nmake -f nmake-evc4.mak clean
echo    nmake -f nmake-evc4.mak
echo    nmake -f nmake-evc4.mak install
echo.
