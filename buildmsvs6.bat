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
@if %ERRORLEVEL% GTR 0 call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32"
@rem Add include and library specifiers to the environment, since for some
@rem reason, VMware execution of a guest command via their api doesn't seem
@rem to allow access to HKCU properly ("default" user probably)
set DEPD=c:\build\sipx-depends
set INCLUDE=%DEPD%\cppunit-1.10.2\include;%DEPD%\pcre\include;%DEPD%\openssl\inc32;%INCLUDE%
set LIB=%DEPD%\cppunit-1.10.2\lib;%DEPD%\pcre\lib;%DEPD%\openssl\out32.dbg;%LIB%

cd sipXportLib
del %releaseType%\sipXportLib%libPrefix%.lib
msdev sipXportLib.dsp /USEENV /MAKE "sipXportLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXportLib
del %releaseType%\sipXportLibTest.exe
msdev sipXportLibTest.dsp /USEENV /MAKE "sipXportLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXsdpLib
del %releaseType%\sipXsdpLib%libPrefix%.lib
msdev sipXsdpLib.dsp /MAKE "sipXsdpLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseType%\sipXtackLib%libPrefix%.lib
msdev sipXtackLib.dsp /USEENV /MAKE "sipXtackLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXsdpLib
del %releaseType%\sipXsdpLibTest.exe
msdev sipXsdpLibTest.dsp /MAKE "sipXsdpLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXtackLib
del %releaseType%\sipXtackLibTest.exe
msdev sipXtackLibTest.dsp /USEENV /MAKE "sipXtackLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseType%\sipXmediaLib%libPrefix%.lib
msdev sipXmediaLib.dsp /USEENV /MAKE "sipXmediaLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaLib
del %releaseType%\sipXmediaLibTest.exe
msdev sipXmediaLibTest.dsp /USEENV /MAKE "sipXmediaLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseType%\sipXmediaAdapterLib%libPrefix%.lib
msdev sipXmediaAdapterLib.dsp /USEENV /MAKE "sipXmediaAdapterLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXmediaAdapterLib
del %releaseType%\sipXmediaAdapterLibTest.exe
msdev sipXmediaAdapterLibTest.dsp /USEENV /MAKE "sipXmediaAdapterLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del %releaseType%\sipXcallLib%libPrefix%.lib
msdev sipXcallLib.dsp /USEENV /MAKE "sipXcallLib - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del %releaseType%\sipXcallLibTest.exe
msdev sipXcallLibTest.dsp /USEENV /MAKE "sipXcallLibTest - Win32 %releaseType%" %doClean%
cd ..

cd sipXcallLib 
del sipXtapi\%releaseType%\sipXtapiTest.exe
msdev sipXtapiTest.dsp /USEENV /MAKE "sipXtapiTest - Win32 %releaseType%" %doClean%
cd ..

goto end

cd sipXcallLib\examples\sipXezPhone 
del Debug\sipXezPhone.exe
msdev sipXezPhone.dsp /USEENV /MAKE "sipXezPhone - Win32 Debug" %doClean%
cd ..\..\..

cd sipXcommserverLib 
del Debug\sipXcommserverLibd.lib
msdev sipXcommserverLib.dsp /USEENV /MAKE "sipXcommserverLib - Win32 Debug" %doClean%
cd ..

cd sipXregistry 
del Debug\sipXregistry.exe
msdev sipXregistry.dsp /USEENV /MAKE "sipXregistry - Win32 Debug" %doClean%
cd ..

cd sipXproxy
del Debug\sipXforkingProxy.exe
msdev sipXforkingProxy.dsp /USEENV /MAKE "sipXforkingProxy - Win32 Debug" %doClean%
cd ..

cd sipXproxy
del Debug\sipXauthproxy.exe
msdev sipXauthproxy.dsp /USEENV /MAKE "sipXauthproxy - Win32 Debug" %doClean%
cd ..

:end
pwd
