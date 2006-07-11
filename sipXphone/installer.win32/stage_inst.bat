@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET PHONE_BASE=".."

:SET_JRE_BASE
  IF NOT "%SIPXPHONE_JRE_BASE%" == "" GOTO CHECK_DIRS
  SET SIPXPHONE_JRE_BASE=C:\Program Files\Java\j2re1.4.2_06

:CHECK_DIRS
  IF NOT EXIST "%SIPXPHONE_JRE_BASE%" GOTO ERROR_JRE

:PREP_STAGING
  rmdir /S /Q staging 2>NUL
  
  mkdir staging
  mkdir staging\meta
  mkdir staging\meta\Skins
  mkdir staging\meta\Skins\Default
  mkdir staging\j2re1.4.2_03
  mkdir staging\bin
  mkdir staging\lib

:POPULATE_STAGING
  xcopy /E /Y /Q "%SIPXPHONE_JRE_BASE%\*.*" .\staging\j2re1.4.2_03\

  copy %PHONE_BASE%\src\launchix\Release\launchix.exe .\staging\bin\
  copy %PHONE_BASE%\QuickConfig\Release\QuickConfig.exe .\staging\bin\

  copy %PHONE_BASE%\softphone\meta\Skins\Default\*.* .\staging\meta\Skins\Default\
  copy %PHONE_BASE%\softphone\meta\*.* .\staging\meta\
  copy meta\quickconfig .\staging\meta\
  del .\staging\meta\cache.ser
  del .\staging\meta\*-config
  
  copy %PHONE_BASE%\softphone\lib\softphone.jar .\staging\lib\
  copy %PHONE_BASE%\javacore\lib\javacore.jar .\staging\lib\
  copy %PHONE_BASE%\lib\*.* .\staging\lib\
  copy %PHONE_BASE%\Release\sipXphone.dll .\staging\bin\

  copy %PHONE_BASE%\COPYING .\meta\license.txt
  COPY %PHONE_BASE%\README .\meta\readme.txt
  copy %PHONE_BASE%\src\pinger\wnt\res\phone.ico .\meta\sipXphone.ico
  GOTO EXIT



:ERROR_GLIB
  ECHO Unable to find GLIB in %SIPXPHONE_GLIB_BASE%, please set SIPXPHONE_GLIB_BASE
  GOTO EXIT

:ERROR_JRE
  ECHO Unable to find JREin %SIPXPHONE_JRE_BASE%, please set SIPXPHONE_JRE_BASE
  GOTO EXIT

:EXIT
