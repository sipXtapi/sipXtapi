@echo Copyright (C) 2005-2006 SIPfoundry Inc.
@echo Licensed by SIPfoundry under the LGPL license.
@echo -
@echo Copyright (C) 2005-2006 SIPez LLC.
@echo Licensed to SIPfoundry under a Contributor Agreement.
@echo -
@echo Building sipX using MS Visual Studio 6.0

@set doClean=
if "%1" == "clean" set doClean="/CLEAN"
@set releaseType=Debug
@set libPrefix=d
@echo set releaseType=Release
@echo set libPrefix

@msdev /? > nul
@rem error level: %ERRORLEVEL%
@if %ERRORLEVEL% GTR 0 call vcvars32

cd sipXportLib
del %releaseType%\sipXportLib%libPrefix%.lib
msdev sipXportLib.dsp /MAKE "sipXportLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXportLib
del %releaseType%\sipXportLibTest.exe
msdev sipXportLibTest.dsp /MAKE "sipXportLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseType%\sipXtackLib%libPrefix%.lib
msdev sipXtackLib.dsp /MAKE "sipXtackLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseType%\sipXtackLibTest.exe
msdev sipXtackLibTest.dsp /MAKE "sipXtackLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseType%\sipXmediaLib%libPrefix%.lib
msdev sipXmediaLib.dsp /MAKE "sipXmediaLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseType%\sipXmediaLibTest.exe
msdev sipXmediaLibTest.dsp /MAKE "sipXmediaLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseType%\sipXmediaAdapterLib%libPrefix%.lib
msdev sipXmediaAdapterLib.dsp /MAKE "sipXmediaAdapterLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseType%\sipXmediaAdapterLibTest.exe
msdev sipXmediaAdapterLibTest.dsp /MAKE "sipXmediaAdapterLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del %releaseType%\sipXcallLib%libPrefix%.lib
msdev sipXcallLib.dsp /MAKE "sipXcallLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del %releaseType%\sipXcallLibTest.exe
msdev sipXcallLibTest.dsp /MAKE "sipXcallLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del sipXtapi\%releaseType%\sipXtapiTest.exe
msdev sipXtapiTest.dsp /MAKE "sipXtapiTest - Win32 %releaseType%" %doClean%
cd ..

goto end

cd sipXcallLib\examples\sipXezPhone 
del Debug\sipXezPhone.exe
msdev sipXezPhone.dsp /MAKE "sipXezPhone - Win32 Debug" %doClean%
cd ..\..\..

cd sipXcommserverLib 
del Debug\sipXcommserverLibd.lib
msdev sipXcommserverLib.dsp /MAKE "sipXcommserverLib - Win32 Debug" %doClean%
cd ..

cd sipXregistry 
del Debug\sipXregistry.exe
msdev sipXregistry.dsp /MAKE "sipXregistry - Win32 Debug" %doClean%
cd ..

cd sipXproxy
del Debug\sipXforkingProxy.exe
msdev sipXforkingProxy.dsp /MAKE "sipXforkingProxy - Win32 Debug" %doClean%
cd ..

cd sipXproxy
del Debug\sipXauthproxy.exe
msdev sipXauthproxy.dsp /MAKE "sipXauthproxy - Win32 Debug" %doClean%
cd ..

:end
pwd
