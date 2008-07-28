@ECHO OFF
REM Copyright 2008 AOL LLC.
REM Licensed to SIPfoundry under a Contributor Agreement.
REM
REM This library is free software; you can redistribute it and/or
REM modify it under the terms of the GNU Lesser General Public
REM License as published by the Free Software Foundation; either
REM version 2.1 of the License, or (at your option) any later version.
REM
REM This library is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
REM Lesser General Public License for more details.
REM
REM You should have received a copy of the GNU Lesser General Public
REM License along with this library; if not, write to the Free Software
REM Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
REM USA. 
REM  
REM Copyright (C) 2004-2006 SIPfoundry Inc.
REM Licensed by SIPfoundry under the LGPL license.
REM
REM Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
REM Licensed to SIPfoundry under a Contributor Agreement.

ECHO.
ECHO ****************************************
ECHO Staging sipXmedia binaries
ECHO ****************************************

SET SOURCE_BASE=..\..\sipXmediaLib
SET STAGE_BASE=..\build.Win32\bin\
mkdir %STAGE_BASE% 2>nul
mkdir %STAGE_BASE%\Debug 2>nul

:POPULATE_STAGING
  copy %SOURCE_BASE%\Release\sipXmediaLib.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXmediaLib.pdb %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXmediaLib.map %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  
  copy %SOURCE_BASE%\Debug\sipXmediaLib.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXmediaLib.pdb %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXmediaLib.map %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\src\mp\codecs\plgpcmapcmu\Release\codec_*.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgilbc\Release\codec_*.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plggsm\Release\codec_*.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgspeex\Release\codec_*.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgtones\Release\codec_*.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

  copy %SOURCE_BASE%\src\mp\codecs\plgpcmapcmu\Debug\codec_*.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgilbc\Debug\codec_*.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plggsm\Debug\codec_*.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgspeex\Debug\codec_*.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\src\mp\codecs\plgtones\Debug\codec_*.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  
GOTO DONE

:ERROR_EXIT
ECHO.
ECHO ****************************************
ECHO *** %0 Error detected, aborting ... ***
ECHO ****************************************
ECHO .
exit /b 1

:DONE
exit /b 0
