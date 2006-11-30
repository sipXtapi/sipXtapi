REM @ECHO OFF

REM  
REM  Copyright (C) 2004, 2005 Pingtel Corp.
REM  
REM

SET SOURCE_BASE=".."

:PREP_STAGING
  rmdir /S /Q staging 2>NUL
  
  mkdir staging
  mkdir staging\bin
  mkdir staging\lib
  mkdir staging\include
  mkdir staging\doc
  mkdir staging\bin\res

:POPULATE_STAGING
  copy %SOURCE_BASE%\..\..\LICENSE.txt .\staging\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXtapi.dll .\staging\bin\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXtapid.dll .\staging\bin\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXtapi.lib .\staging\lib\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXtapid.lib .\staging\lib\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\..\examples\bin\PlaceCall.exe .\staging\bin\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\..\examples\bin\ReceiveCall.exe .\staging\bin\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\Release\sipXtapiTest.exe .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\..\doc\sipXtapi\html\* .\staging\doc\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  
  copy %SOURCE_BASE%\..\include\tapi\sipXtapi.h .\staging\include\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\..\include\tapi\sipXtapiEvents.h .\staging\include\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  
  copy \windows\system32\msvcr71d.dll .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy \windows\system32\msvcr71.dll .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\..\examples\sipXezPhone\Release\sipXezPhone.exe .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\..\examples\sipXezPhone\res\*.* .\staging\bin\res
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXtapid.pdb .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
REM  copy %SOURCE_BASE%\Release\sipXtapi.pdb .\staging\bin
REM  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\Debug\*.map .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\*.map .\staging\bin
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .
exit /b 1

:DONE
exit /b 0
