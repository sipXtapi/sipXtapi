# Microsoft Developer Studio Project File - Name="ReceiveCall" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ReceiveCall - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReceiveCall.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReceiveCall.mak" CFG="ReceiveCall - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReceiveCall - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ReceiveCall - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReceiveCall - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReceiveCall___Win32_Release"
# PROP BASE Intermediate_Dir "ReceiveCall___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReceiveCall___Win32_Release"
# PROP Intermediate_Dir "ReceiveCall___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\sipXmediaAdapterLib\interface" /I "..\..\include" /I "include" /I "..\PlaceCall\include" /I "..\..\..\sipXportLib\include" /I "..\..\..\sipXtackLib\include" /I "..\..\..\sipXsdpLib\include" /I "..\..\..\sipXportLib\src\test" /I "..\..\..\CPPUnit\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SIPXTAPI_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib sipXsdpLib.lib sipXtackLib.lib sipXportLib.lib sipXmediaLib.lib sipXmediaAdapterLib.lib sipXcallLib.lib cppunit_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib wsock32.lib /nologo /subsystem:console /machine:I386 /out:"Release/ReceiveCall.exe" /libpath:"..\sipXportLib\Release" /libpath:"..\..\..\sipXsdpLib\Release" /libpath:"..\..\..\sipXtackLib\Release" /libpath:"..\..\..\sipXmediaLib\Release" /libpath:"..\..\..\sipXmediaAdapterLib\Release" /libpath:"..\..\..\sipXcallLib\Release" /libpath:"..\..\..\CPPUnit\lib" /libpath:"..\..\..\sipXmediaLib\contrib\libspeex\lib" /libpath:"..\..\..\sipXmediaLib\contrib\libgsm\lib"

!ELSEIF  "$(CFG)" == "ReceiveCall - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ReceiveCall___Win32_Debug"
# PROP BASE Intermediate_Dir "ReceiveCall___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ReceiveCall___Win32_Debug"
# PROP Intermediate_Dir "ReceiveCall___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\sipXmediaAdapterLib\interface" /I "..\..\include" /I "include" /I "..\PlaceCall\include" /I "..\..\..\sipXportLib\include" /I "..\..\..\sipXtackLib\include" /I "..\..\..\sipXsdpLib\include" /I "..\..\..\sipXportLib\src\test" /I "..\..\..\CPPUnit\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "SIPXTAPI_STATIC" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib sipXportLibd.lib sipXsdpLibd.lib sipXtackLibd.lib sipXmediaAdapterLibd.lib sipXmediaLibd.lib sipXcallLibd.lib wsock32.lib cppunitd_dll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pcre.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/ReceiveCall.exe" /pdbtype:sept /libpath:"..\..\..\sipXportLib\Debug" /libpath:"..\..\..\sipXsdpLib\Debug" /libpath:"..\..\..\sipXtackLib\Debug" /libpath:"..\..\..\sipXmediaLib\Debug" /libpath:"..\..\..\sipXmediaAdapterLib\Debug" /libpath:"..\..\..\sipXcallLib\Debug" /libpath:"..\..\..\CPPUnit\lib" /libpath:"..\..\..\sipXmediaLib\contrib\libspeex\lib" /libpath:"..\..\..\sipXmediaLib\contrib\libgsm\lib"

!ENDIF 

# Begin Target

# Name "ReceiveCall - Win32 Release"
# Name "ReceiveCall - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "RecieveCall"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PlaceCall\src\ExternalTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaceCall\include\ExternalTransport.h
# End Source File
# Begin Source File

SOURCE=.\src\ReceiveCall.cpp
# End Source File
# Begin Source File

SOURCE=.\include\ReceiveCall.h
# End Source File
# Begin Source File

SOURCE=.\src\ReceiveCallWntApp.cpp
# End Source File
# Begin Source File

SOURCE=.\include\ReceiveCallWntApp.h
# End Source File
# End Group
# End Target
# End Project
