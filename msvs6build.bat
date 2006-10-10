@echo Copyright (C) 2005-2006 SIPfoundry Inc.
@echo Licensed by SIPfoundry under the LGPL license.
@echo -
@echo Copyright (C) 2005-2006 SIPez LLC.
@echo Licensed to SIPfoundry under a Contributor Agreement.
@echo -
@echo Building sipX using MS Visual Studio 6.0

@set doClean=
if "%1" == "clean" set doClean="/CLEAN"

@msdev /? > nul
@rem error level: %ERRORLEVEL%
@if %ERRORLEVEL% GTR 0 vcvars32

cd sipXportLib
del Debug\sipXportLibd.lib
msdev sipXportLib.dsp /MAKE "sipXportLib - Win32 Debug" %doClean%
cd ..

cd sipXportLib
del Debug\sipXportLibTest.exe
msdev sipXportLibTest.dsp /MAKE "sipXportLibTest - Win32 Debug" %doClean%
cd ..

cd sipXtackLib
del Debug\sipXtackLibd.lib
msdev sipXtackLib.dsp /MAKE "sipXtackLib - Win32 Debug" %doClean%
cd ..

cd sipXtackLib
del Debug\sipXtackLibTest.exe
msdev sipXtackLibTest.dsp /MAKE "sipXtackLibTest - Win32 Debug" %doClean%
cd ..

cd sipXmediaLib
del Debug\sipXmediaLibd.lib
msdev sipXmediaLib.dsp /MAKE "sipXmediaLib - Win32 Debug" %doClean%
cd ..

cd sipXmediaAdapterLib
del Debug\sipXmediaAdapterLibd.lib
msdev sipXmediaAdapterLib.dsp /MAKE "sipXmediaAdapterLib - Win32 Debug" %doClean%
cd ..

cd sipXmediaAdapterLib
del Debug\sipXmediaAdapterLibTest.exe
msdev sipXmediaAdapterLibTest.dsp /MAKE "sipXmediaAdapterLibTest - Win32 Debug" %doClean%
cd ..

cd sipXcallLib 
del Debug\sipXcallLibd.lib
msdev sipXcallLib.dsp /MAKE "sipXcallLib - Win32 Debug" %doClean%
cd ..

cd sipXcallLib 
del Debug\sipXcallLibTest.exe
msdev sipXcallLibTest.dsp /MAKE "sipXcallLibTest - Win32 Debug" %doClean%
cd ..

cd sipXcallLib 
del sipXtapi\Debug\sipXtapiTest.exe
msdev sipXtapiTest.dsp /MAKE "sipXtapiTest - Win32 Debug" %doClean%
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
