@echo off
echo Copyright (C) 2006 SIPfoundry Inc.
echo Licensed by SIPfoundry under the LGPL license.
echo -
echo Copyright (C) 2006 SIPez LLC.
echo Licensed to SIPfoundry under a Contributor Agreement.
echo -
echo Building sipX using MS eMbedded Visual C++ 4.0

set doClean=
if "%1" == "clean" set doClean="/CLEAN"
if "%releaseType%"=="" set releaseType=Debug
if "%releaseType%"=="Debug" set releaseDir=%TARGETCPU%Dbg
if "%releaseType%"=="Release" set releaseDir=%TARGETCPU%Rel

REM Not implemented for EVC yet if "%releaseType%"=="Debug" set libPrefix=d
echo set releaseType=Release
REM Not implemented for EVC yet echo set libPrefix

evc /? >nul 2>nul
rem error level: %ERRORLEVEL%
if %ERRORLEVEL% GTR 0 call wcevars.bat

echo on
cd sipXportLib
del %releaseDir%\sipXportLib%libPrefix%.lib 2>nul
evc sipXportLib.vcp /MAKE "sipXportLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXportLib
del %releaseDir%\sipXportLibTest.exe 2>nul
evc sipXportLibTest.vcp /MAKE "sipXportLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXsdpLib
del %releaseDir%\sipXsdpLib%libPrefix%.lib 2>nul
evc sipXsdpLib.vcp /MAKE "sipXsdpLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseDir%\sipXtackLib%libPrefix%.lib 2>nul
evc sipXtackLib.vcp /MAKE "sipXtackLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXsdpLib
del %releaseDir%\sipXsdpLibTest.exe 2>nul
evc sipXsdpLibTest.vcp /MAKE "sipXsdpLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseDir%\sipXtackLibTest.exe 2>nul
evc sipXtackLibTest.vcp /MAKE "sipXtackLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseDir%\sipXmediaLib%libPrefix%.lib 2>nul
evc sipXmediaLib.vcp /MAKE "sipXmediaLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseDir%\sipXmediaLibTest%libPrefix%.lib 2>nul
evc sipXmediaLibTest.vcp /MAKE "sipXmediaLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseDir%\sipXmediaAdapterLib%libPrefix%.lib 2>nul
evc sipXmediaAdapterLib.vcp /MAKE "sipXmediaAdapterLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseDir%\sipXmediaAdapterLibTest.exe 2>nul
evc sipXmediaAdapterLibTest.vcp /MAKE "sipXmediaAdapterLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

@goto end

cd sipXcallLib 
del %releaseDir%\sipXcallLib%libPrefix%.lib 2>nul
evc sipXcallLib.vcp /MAKE "sipXcallLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del %releaseDir%\sipXcallLibTest.exe 2>nul
evc sipXcallLibTest.vcp /MAKE "sipXcallLibTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del sipXtapi\%releaseDir%\sipXtapiTest.exe 2>nul
evc sipXtapiTest.vcp /MAKE "sipXtapiTest - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXcallLib\examples\sipXezPhone 
del %releaseDir%\sipXezPhone.exe 2>nul
evc sipXezPhone.vcp /MAKE "sipXezPhone - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..\..\..

cd sipXcommserverLib 
del %releaseDir%\sipXcommserverLibd.lib 2>nul
evc sipXcommserverLib.vcp /MAKE "sipXcommserverLib - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXregistry 
del %releaseDir%\sipXregistry.exe 2>nul
evc sipXregistry.vcp /MAKE "sipXregistry - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXproxy
del %releaseDir%\sipXforkingProxy.exe 2>nul
evc sipXforkingProxy.vcp /MAKE "sipXforkingProxy - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

cd sipXproxy
del %releaseDir%\sipXauthproxy.exe 2>nul
evc sipXauthproxy.vcp /MAKE "sipXauthproxy - Win32 (WCE %TARGETCPU%) %releaseType%" %doClean%
cd ..

@:end
