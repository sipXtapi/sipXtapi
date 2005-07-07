@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET SOURCE_BASE=".."

IF NOT "%SIPXPHONE_GLIB_BASE%" == "" GOTO CHECK_DIRS
:SET_GLIB_BASE
  SET SIPXPHONE_GLIB_BASE=C:\GLIB

:CHECK_DIRS
  IF NOT EXIST "%SIPXPHONE_GLIB_BASE%" GOTO ERROR_GLIB

:PREP_STAGING
  rmdir /S /Q staging 2>NUL
  
  mkdir staging
  mkdir staging\bin
  mkdir staging\lib
  mkdir staging\include
  mkdir staging\doc

:POPULATE_STAGING
  copy "%SIPXPHONE_GLIB_BASE%\bin\libglib-2.0-0.dll" .\staging\bin\
  copy "%SIPXPHONE_GLIB_BASE%\bin\libgthread-2.0-0.dll" .\staging\bin\
  copy "%SIPXPHONE_GLIB_BASE%\bin\iconv.dll" .\staging\bin\
  copy "%SIPXPHONE_GLIB_BASE%\bin\intl.dll" .\staging\bin\

  copy %SOURCE_BASE%\Release\sipXtapi.dll .\staging\bin\
  copy %SOURCE_BASE%\Debug\sipXtapid.dll .\staging\bin\
  copy %SOURCE_BASE%\Release\sipXtapi.lib .\staging\lib\
  copy %SOURCE_BASE%\Debug\sipXtapid.lib .\staging\lib\

  copy %SOURCE_BASE%\..\examples\PlaceCall\Release\PlaceCall.exe .\staging\bin\
  copy %SOURCE_BASE%\..\examples\ReceiveCall\Release\ReceiveCall.exe .\staging\bin\

  copy %SOURCE_BASE%\Release\sipXtapiTest.exe .\staging\bin

  copy %SOURCE_BASE%\..\doc\sipXtapi\html\* .\staging\doc\
  
  copy %SOURCE_BASE%\..\include\tapi\sipXtapi.h .\staging\include\
  copy %SOURCE_BASE%\..\include\tapi\sipXtapiEvents.h .\staging\include\
  GOTO EXIT

:ERROR_GLIB
  ECHO Unable to find GLIB in %SIPXPHONE_GLIB_BASE%, please set SIPXPHONE_GLIB_BASE
  GOTO EXIT

:ERROR_JRE
  ECHO Unable to find JREin %SIPXPHONE_JRE_BASE%, please set SIPXPHONE_JRE_BASE
  GOTO EXIT

:EXIT
