@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

IF NOT "%SIPXPHONE_INSTALLSHIELD_BASE%" == "" GOTO BUILD_INSTALLER
SET SIPXPHONE_INSTALLSHIELD_BASE=C:\Program Files\InstallShield X Express\System

:BUILD_INSTALLER
  "%SIPXPHONE_INSTALLSHIELD_BASE%\IsExpCmdBld" -p sipXphone.ise
