# Microsoft Developer Studio Project File - Name="sipXtapiDriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXtapiDriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapiDriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXtapiDriver.mak" CFG="sipXtapiDriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXtapiDriver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXtapiDriver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXtapiDriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sipXtapiDriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "include" /I "..\..\..\sipXportLib\include" /I "..\..\..\sipXportLib\include\glib" /I "..\..\..\sipXtackLib\include" /I "..\..\..\sipXcallLib\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "DISABLE_MEM_POOLS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 sipXtapid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib sipXportLib.lib sipXtackLib.lib libpcre.a /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"Debug" /libpath:"..\..\..\sipXportLib\Debug" /libpath:"..\..\..\sipXtackLib\Debug" /libpath:"..\..\sipXtapi\Debug"

!ENDIF 

# Begin Target

# Name "sipXtapiDriver - Win32 Release"
# Name "sipXtapiDriver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\sipXtapiDriver\AddLineCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\AutoAnswerCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\AutoRedirectCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\AutoRejectCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallAcceptCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallAnswerCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallConnectCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallCreateCommmand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallDestroyCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallHoldCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallPlayFileCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallRedirectCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallSendInfoCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallStartToneCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CallSubscribeCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\Command.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CommandMsgProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CommandProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceAddCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceCreateCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceDestroyCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceHoldCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceJoinCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ConferenceUnholdCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\CreatePublisherCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\DestroyPublisherCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\EnableRportCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\EnableStunCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\ExitCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\HelpCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\HistoryCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\sipXtapiDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\SleepCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipXtapiDriver\UpdatePublisherCommand.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\sipXtapiDriver\AddLineCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\AutoAnswerCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\AutoRedirectCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\AutoRejectCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallAcceptCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallAnswerCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallConnectCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallCreateCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallDestroyCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallHoldCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallPlayFileCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallRedirectCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallSendInfoCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallStartToneCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CallSubscribeCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\Command.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CommandMsgProcessor.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CommandProcessor.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceAddCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceCreateCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceDestroyCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceHoldCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceJoinCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ConferenceUnHoldCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\CreatePublisherCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\DestroyPublisherCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\EnableRportCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\EnableStunCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\ExitCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\HelpCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\HistoryCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\SleepCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\sipXtapiDriver\UpdatePublisherCommand.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
