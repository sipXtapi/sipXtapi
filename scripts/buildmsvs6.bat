@echo Copyright (C) 2005-2006 SIPfoundry Inc.
@echo Licensed by SIPfoundry under the LGPL license.
@echo -
@echo Copyright (C) 2005-2006 SIPez LLC.
@echo Licensed to SIPfoundry under a Contributor Agreement.
@echo -
@echo Building sipX using MS Visual Studio 6.0

@set doClean=
if "%1" == "clean" set doClean="/CLEAN"
if "%1" == "check" goto run_unittests
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
set INCLUDE=%DEPD%\cppunit-1.10.2\include;%DEPD%\pcre\include;%DEPD%\openssl\inc32;%DEPD%\libspeex\include;%INCLUDE%
set LIB=%DEPD%\cppunit-1.10.2\lib;%DEPD%\pcre\lib;%DEPD%\openssl\out32.dbg;%DEPD%\libgsm\lib;%DEPD%\libspeex\lib;%LIB%

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

cd sipXmediaLib\contrib\libgsm
del %releaseType%\gsm%libPrefix%.lib
msdev libgsm.dsp /USEENV /MAKE "libgsm - Win32 %releaseType%" %doClean%
cd ..\..\..

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
del %releaseType%\sipXtapiTest.exe
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

goto end

:run_unittests
REM Here we run the unit tests
@echo off
set DEPD=C:\build\sipx-depends
set PATH=c:\Program Files\GnuWin32\bin;%DEPD%\cppunit-1.10.2\lib;%DEPD%\pcre\bin;%DEPD%\openssl\out32.dbg;%DEPD%\libgsm\lib;%DEPD%\libspeex\lib;%PATH%

set NAME_PREFIX=test-test-
if not "%1" == "" set NAME_PREFIX=%1
echo logging in %NAME_PREFIX% files
svn info | grep Revision

sipXportLib\Debug\sipXportLibTest.exe 2> %NAME_PREFIX%port.err.txt > %NAME_PREFIX%port.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXportLib - /" %NAME_PREFIX%port.txt

sipXtackLib\Debug\sipXtackLibTest.exe 2> %NAME_PREFIX%stack.err.txt > %NAME_PREFIX%stack.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtackLib - /" %NAME_PREFIX%stack.txt

sipXmediaLib\Debug\sipXmediaLibTest.exe 2> %NAME_PREFIX%media.err.txt > %NAME_PREFIX%media.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXmediaLib - /" %NAME_PREFIX%media.txt

sipXmediaAdapterLib\Debug\sipXmediaAdapterLibTest.exe 2> %NAME_PREFIX%adapter.err.txt > %NAME_PREFIX%adapter.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXmediaAdapterLib - /" %NAME_PREFIX%adapter.txt

sipXcallLib\Debug\sipXcallLibTest.exe 2> %NAME_PREFIX%call.err.txt > %NAME_PREFIX%call.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXcallLib - /" %NAME_PREFIX%call.txt

sipXcallLib\Debug\sipXtapiTest.exe 2> %NAME_PREFIX%tapi.err.txt > %NAME_PREFIX%tapi.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtapi - /" %NAME_PREFIX%tapi.txt
sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtapi - /" %NAME_PREFIX%tapi.err.txt            

:end
REM Print the current directory
cd
