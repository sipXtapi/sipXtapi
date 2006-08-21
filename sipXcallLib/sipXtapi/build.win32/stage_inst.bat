@ECHO OFF

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
  copy %SOURCE_BASE%\Release\sipXtapi.dll .\staging\bin\
  copy %SOURCE_BASE%\Debug\sipXtapid.dll .\staging\bin\
  copy %SOURCE_BASE%\Release\sipXtapi.lib .\staging\lib\
  copy %SOURCE_BASE%\Debug\sipXtapid.lib .\staging\lib\

  copy %SOURCE_BASE%\..\examples\bin\PlaceCall.exe .\staging\bin\
  copy %SOURCE_BASE%\..\examples\bin\ReceiveCall.exe .\staging\bin\

  copy %SOURCE_BASE%\Release\sipXtapiTest.exe .\staging\bin

  copy %SOURCE_BASE%\..\doc\sipXtapi\html\* .\staging\doc\
  
  copy %SOURCE_BASE%\..\include\tapi\sipXtapi.h .\staging\include\
  copy %SOURCE_BASE%\..\include\tapi\sipXtapiEvents.h .\staging\include\
  
  copy \windows\system32\msvcr71d.dll .\staging\bin
  copy \windows\system32\msvcr71.dll .\staging\bin

  copy %SOURCE_BASE%\..\examples\sipXezPhone\sipXezPhone.exe .\staging\bin

  copy %SOURCE_BASE%\..\examples\sipXezPhone\res\*.* .\staging\bin\res
  copy %SOURCE_BASE%\Debug\sipXtapid.pdb .\staging\bin
  copy %SOURCE_BASE%\Release\sipXtapi.pdb .\staging\bin

  copy %SOURCE_BASE%\Debug\*.map .\staging\bin
  copy %SOURCE_BASE%\Release\*.map .\staging\bin
  
  GOTO EXIT

:EXIT
