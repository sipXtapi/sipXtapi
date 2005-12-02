@ECHO OFF

REM  
REM  Copyright (C) 2004 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2004 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET LAUNCHIX=..\..\src\launchix\Debug\launchix.exe
SET CHECKCFG=..\..\src\CheckCfg\Debug\CheckCfg.exe
SET NATIVELIB=..\..\Debug\sipXphone.dll
SET SKINPATH=..\meta\Skins\Default
SET OPTIONS=-ms6700000 -Dsoftphone.library.path=%NATIVELIB% -Dsoftphone.skin.path=%SKINPATH%

SET JAVACORE_JAR=..\..\javacore\lib\javacore.jar
SET SOFTPHONE_JAR=..\lib\softphone.jar
SET JTAPI_JAR=..\..\lib\jtapi13.jar
SET JNDI_JAR=..\..\jndi1.2.1.jar
SET LDAP_JAR=..\..\ldap.jar
SET JUNIT_JAR=..\..\junit.jar
SET OROMATCH_JAR=..\..\oromatch.jar

SET SIPXCLASSPATH=%JAVACORE_JAR%;%SOFTPHONE_JAR%;%JTAPI_JAR%;%JNDI_JAR%;%LDAP_JAR%;%JUNIT_JAR%;%OROMATCH_JAR%

SET MAINCLASS=org.sipfoundry.sipxphone.testbed.TestbedFrame 

IF EXISTS "%CHECKCFG%" "%CHECKCFG%"

start %LAUNCHIX% javaw %OPTIONS% -Xbootclasspath/a:%SIPXCLASSPATH% %MAINCLASS% %NATIVELIB%
