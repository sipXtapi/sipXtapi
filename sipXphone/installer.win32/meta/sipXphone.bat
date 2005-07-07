@ECHO OFF

REM  
REM  Copyright (C) 2004-2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2004-2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

CD Meta

SET BASE_PATH=%CD%

SET LAUNCHIX=..\bin\launchix.exe
SET QUICKCONFIG="%BASE_PATH%\..\bin\QuickConfig.exe"
SET NATIVELIB="%BASE_PATH%\..\bin\sipXphone.dll"
SET SKINPATH="%BASE_PATH%\Skins\Default"
SET OPTIONS=-ms6700000 -Dsoftphone.library.path=%NATIVELIB% -Dsoftphone.skin.path=%SKINPATH%
SET JVM_PATH="%BASE_PATH%\..\j2re1.4.2_03\bin\javaw.exe"

SET JAVACORE_JAR=..\lib\javacore.jar
SET SOFTPHONE_JAR=..\lib\softphone.jar
SET JTAPI_JAR=..\lib\jtapi13.jar
SET JNDI_JAR=..\lib\jndi1.2.1.jar
SET LDAP_JAR=..\lib\ldap.jar
SET JUNIT_JAR=..\lib\junit.jar
SET OROMATCH_JAR=..\lib\oromatch.jar

SET SIPXCLASSPATH=%JAVACORE_JAR%;%SOFTPHONE_JAR%;%JTAPI_JAR%;%JNDI_JAR%;%LDAP_JAR%;%JUNIT_JAR%;%OROMATCH_JAR%
SET MAINCLASS=org.sipfoundry.sipxphone.testbed.TestbedFrame 

SET SIPX_OLD_PATH=%PATH%
SET PATH=%PATH%;..\bin

REM Call QuickConfig if included and the pinger-config doesn't exist
IF NOT EXIST "quickconfig" GOTO START_IX
IF EXIST "pinger-config" GOTO START_IX
%QUICKCONFIG%

:START_IX
start %LAUNCHIX% %JVM_PATH% %OPTIONS% -Xbootclasspath/a:%SIPXCLASSPATH% %MAINCLASS%
CD ..

SET PATH=%SIPX_OLD_PATH%
