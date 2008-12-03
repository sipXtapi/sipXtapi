option explicit

'determine if we're running in cscript mode, and bitch if not.
if UCase(Right(WScript.FullName, 11)) <> "CSCRIPT.EXE" then
   WScript.echo("Running autobuild in wscript mode unsupported." & vbCrLf _
         & "Please re-run using cscript")
   WScript.Quit(1)
end if

WScript.echo "Copyright (C) 2005-2008 SIPfoundry Inc."
WScript.echo "Licensed by SIPfoundry under the LGPL license."
WScript.echo ""
WScript.echo "Copyright (C) 2005-2008 SIPez LLC."
WScript.echo "Licensed to SIPfoundry under a Contributor Agreement."
WScript.echo ""
WScript.echo "Building sipX using MS Visual Studio"


' Put this class and it's primitive singleton-like initializer
' at the top of your script.
' This class makes sure that wsh/vbscript runtime errors are caught
' and returned as ERRORLEVEL when the script exits.  If you don't
' do this, runtime errors will still result in the caller of this
' script receiving 0 (success) error code.
Class QuitErrorlevelOnRuntimeError
   Private Sub Class_Terminate()
      ' When 'WScript.Quit' is called, it seems to set err.Number 
      ' to -2147155971.  Since we want the quit to return it's 
      ' error code, don't return err.number if this err.number is 
      ' seen
      dim quitErrNum : quitErrNum = -2147155971
      if Err.Number <> 0 and Err.Number <> quitErrNum then
        WScript.Quit Err.Number
      end if 
   End Sub
End Class
Dim qeloreObj : Set qeloreObj = New QuitErrorlevelOnRuntimeError



dim objShell, objFS, env, args, doClean, releaseType, libPrefix
dim cDir, vcVer

set objShell = CreateObject("WScript.Shell")
set objFS = CreateObject("Scripting.FileSystemObject")
set env = objShell.environment("process")
set args = WScript.Arguments
cDir = objShell.CurrentDirectory

doClean = ""
if args.Count > 0 then
   select case args.Item(0)
   case "clean"
      doClean="/CLEAN"
   case "check"
      dim unittest_errcode
      unittest_errcode = runUnittests()
      WScript.Quit(unittest_errcode)
   case else
      WScript.echo "Invalid arguments!"
      WScript.Quit(1)
   end select
end if

releaseType="Debug"
libPrefix="d"
WScript.echo "releaseType=" & releaseType
WScript.echo "libPrefix=" & libPrefix

Dim vc6vcvarspath, vc8vcvarspath

vc6vcvarspath = "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat"
vc8vcvarspath = "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"

dim msdevpath
msdevpath = getFullExePath("msdev")
' Determine if msdev is in the path
if msdevpath = "" then
   WScript.echo "couldn't find msdev in default path"
   ' Check to see if the msvc6 compiler setup batch file exists (thus msvc6 is installed)
   if objFS.FileExists(vc6vcvarspath) then
      vcVer="6.0"
      WScript.echo "Found vc6 vcvars, calling it and pulling in it's env vars"
      ' Try to call vcvars.
      callBatchfile(vc6vcvarspath)


   ' Check to see if the msvc8/vs2005 compiler setup batch file exists (thus msvc8/vs2005 is installed)
   elseif objFS.FileExists(vc8vcvarspath) then
      vcVer="8.0"
      WScript.echo "Found vc8 vcvars, calling it and pulling in it's env vars"
      ' Try to call vcvars.
      callBatchfile(vc8vcvarspath)

   else
      ' Otherwise, exit without going any further -- we don't have 
      ' any known compiler to do compiling with.
      WScript.echo("Couldn't find a way to access the compiler!")
      WScript.Quit(2)
   end if
else
   WScript.echo "msdevpath = " & msdevpath
   vcVer="6.0"
end if


' For compile steps, if we're compiling using vc6, then 'msdev' is used for
' compiling, else if compiling with vc8, 'devenv' is used.

' For now we say vc8 isn't supported, since priority is on getting vc6 working.
' Only vc6 is currently supported.
'if vcVer = "8.0" then
'   WScript.stderr.writeline("Visual Studio 2005 is not currently supported")
'   WScript.Quit(3)
'elseif vcVer <> "6.0" then
'   WScript.stderr.writeline("Only Visual Studio 6.0 is currently supported")
'   WScript.Quit(3)
'end if 

'WScript.echo("PATH = " & objShell.environment("process")("PATH")) ' Debug
'WScript.Quit(0) ' Debug

dim compileExec, errCode
if vcVer = "6.0" then
   dim staticlibfile
   staticlibfile = releaseType & "\sipXportLib" & libPrefix & ".lib"
   objShell.CurrentDirectory = cDir & "\sipXportLib"
   if objFS.FileExists(staticlibfile) then
      objFS.deleteFile(staticlibfile)
   end if
   errCode = runWithOutput("msdev sipXportLib.dsp /USEENV /MAKE ""sipXportLib - Win32 " & releaseType & """" & doClean)

elseif vcVer = "8.0" then
   dim compileMode
   compileMode = "/build"
   if doClean <> "" then
      compileMode = "/rebuild"
   end if 
   ' Devenv.com needs to be specified specifically, so that output will
   ' go to the console
   errCode = runWithOutput("devenv.com sipX-msvc8.sln /Project ""sipXportLib-msvc8"" /ProjectConfig """ & releaseType & "|Win32"" " & compileMode)
end if 


'cd sipXportLib
'del %releaseType%\sipXportLib%libPrefix%.lib
'msdev sipXportLib.dsp /USEENV /MAKE "sipXportLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXportLib
'del %releaseType%\sipXportLibTest.exe
'msdev sipXportLibTest.dsp /USEENV /MAKE "sipXportLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXsdpLib
'del %releaseType%\sipXsdpLib%libPrefix%.lib
'msdev sipXsdpLib.dsp /MAKE "sipXsdpLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXtackLib
'del %releaseType%\sipXtackLib%libPrefix%.lib
'msdev sipXtackLib.dsp /USEENV /MAKE "sipXtackLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXsdpLib
'del %releaseType%\sipXsdpLibTest.exe
'msdev sipXsdpLibTest.dsp /MAKE "sipXsdpLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXtackLib
'del %releaseType%\sipXtackLibTest.exe
'msdev sipXtackLibTest.dsp /USEENV /MAKE "sipXtackLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXmediaLib
'del %releaseType%\sipXmediaLib%libPrefix%.lib
'msdev sipXmediaLib.dsp /USEENV /MAKE "sipXmediaLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXmediaLib\contrib\libgsm
'del %releaseType%\gsm%libPrefix%.lib
'msdev libgsm.dsp /USEENV /MAKE "libgsm - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..\..\..
'
'cd sipXmediaLib
'del %releaseType%\sipXmediaLibTest.exe
'msdev sipXmediaLibTest.dsp /USEENV /MAKE "sipXmediaLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXmediaAdapterLib
'del %releaseType%\sipXmediaAdapterLib%libPrefix%.lib
'msdev sipXmediaAdapterLib.dsp /USEENV /MAKE "sipXmediaAdapterLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXmediaAdapterLib
'del %releaseType%\sipXmediaAdapterLibTest.exe
'msdev sipXmediaAdapterLibTest.dsp /USEENV /MAKE "sipXmediaAdapterLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXcallLib 
'del %releaseType%\sipXcallLib%libPrefix%.lib
'msdev sipXcallLib.dsp /USEENV /MAKE "sipXcallLib - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXcallLib 
'del %releaseType%\sipXcallLibTest.exe
'msdev sipXcallLibTest.dsp /USEENV /MAKE "sipXcallLibTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXcallLib 
'del %releaseType%\sipXtapiTest.exe
'msdev sipXtapiTest.dsp /USEENV /MAKE "sipXtapiTest - Win32 %releaseType%" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'goto end
'
'cd sipXcallLib\examples\sipXezPhone 
'del Debug\sipXezPhone.exe
'msdev sipXezPhone.dsp /USEENV /MAKE "sipXezPhone - Win32 Debug" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..\..\..
'
'cd sipXcommserverLib 
'del Debug\sipXcommserverLibd.lib
'msdev sipXcommserverLib.dsp /USEENV /MAKE "sipXcommserverLib - Win32 Debug" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXregistry 
'del Debug\sipXregistry.exe
'msdev sipXregistry.dsp /USEENV /MAKE "sipXregistry - Win32 Debug" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXproxy
'del Debug\sipXforkingProxy.exe
'msdev sipXforkingProxy.dsp /USEENV /MAKE "sipXforkingProxy - Win32 Debug" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'cd sipXproxy
'del Debug\sipXauthproxy.exe
'msdev sipXauthproxy.dsp /USEENV /MAKE "sipXauthproxy - Win32 Debug" %doClean%
'@if %ERRORLEVEL% GTR 0 goto end
'cd ..
'
'goto end
'
':run_unittests
'REM Here we run the unit tests
'@echo off
'
'set NAME_PREFIX=test-test-
'if not "%1" == "" set NAME_PREFIX=%1
'echo logging in %NAME_PREFIX% files
'svn info | grep Revision
'
'sipXportLib\Debug\sipXportLibTest.exe 2> %NAME_PREFIX%port.err.txt > %NAME_PREFIX%port.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXportLib - /" %NAME_PREFIX%port.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
'sipXtackLib\Debug\sipXtackLibTest.exe 2> %NAME_PREFIX%stack.err.txt > %NAME_PREFIX%stack.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtackLib - /" %NAME_PREFIX%stack.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
'sipXmediaLib\Debug\sipXmediaLibTest.exe 2> %NAME_PREFIX%media.err.txt > %NAME_PREFIX%media.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXmediaLib - /" %NAME_PREFIX%media.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
'sipXmediaAdapterLib\Debug\sipXmediaAdapterLibTest.exe 2> %NAME_PREFIX%adapter.err.txt > %NAME_PREFIX%adapter.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXmediaAdapterLib - /" %NAME_PREFIX%adapter.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
'sipXcallLib\Debug\sipXcallLibTest.exe 2> %NAME_PREFIX%call.err.txt > %NAME_PREFIX%call.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXcallLib - /" %NAME_PREFIX%call.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
'sipXcallLib\Debug\sipXtapiTest.exe 2> %NAME_PREFIX%tapi.err.txt > %NAME_PREFIX%tapi.txt
'set EL_STORE=%ERRORLEVEL%
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtapi - /" %NAME_PREFIX%tapi.txt
'sed -e "/^\(OK (\|Run:\)/!d;s/^/sipXtapi - /" %NAME_PREFIX%tapi.err.txt
'set ERRORLEVEL=%EL_STORE%
'@if %ERRORLEVEL% GTR 0 goto end
'
':end

Function runUnittests()
   WScript.echo("Function runUnittests running")
End Function

Function callBatchfile(batchFile)
   ' Script pulled as a powershell script from:
   ' http://www.tavaresstudios.com/Blog/post/The-last-vsvars32ps1-Ill-ever-need.aspx
   ' Which got it from the book "Windows Powershell in Action" by Bruce Payette

   ' Ported code to wsh vbscript by Keith Kyzivat
   ' TODO: Still needs to be converted from powershell script to 
   ' WScript/vbscript.
   dim batchExec
   ' below doesn't work by itself, despite what the above referenced page
   ' says.  Try it yourself -- open shell window and type 
   '    cmd /c <path to batch> & set
   ' and see that it *doesn't* print the env vars set in the batch file :(
   ' set batchExec = objShell.Exec("%COMSPEC% /c " & batchFile & " & set")
   
   ' Create a new batch file to call the referenced batch file and then
   ' print out the env vars
   dim f, fStream
   f = "tmp" & int(rnd * 1000) & ".bat"
   set fStream = objFS.CreateTextFile(f, TRUE)
   fStream.writeline("call """ & batchFile & """ >nul")
   fStream.writeline("set")
   fStream.close()
   set fStream = Nothing
   

   set batchExec = objShell.Exec(f)
   ' Wait till the exec is finished
   do while batchExec.status = 0
      wscript.sleep 100
   loop

   ' Can delete the temporary batch file now.
   objFS.deleteFile(f)
   f=""

   dim line
   do while not batchExec.stdout.AtEndOfStream
      line = batchExec.stdout.readline
      if InStr(line, "=") > 0 then
         dim splitline
         splitline=Split(line, "=", 2)
         'WScript.echo("setting " & splitline(0) & "=" & splitline(1))
         env(splitline(0)) = splitline(1)
      end if 
   loop
   set batchExec = Nothing
End Function

Function getFullExePath(exe)
   dim f, fStream
   f = "which" & int(rnd * 1000) & ".bat"
   set fStream = objFS.CreateTextFile(f, FALSE)
   fStream.writeline("@set argonefullpath=%~$PATH:1")
   fStream.writeline("@if ""%argonefullpath%"" == """" goto end")
   fStream.writeline("@echo %argonefullpath%")
   fStream.writeline(":end")
   fStream.close()
   set fStream = Nothing

   dim batchExec
   set batchExec = objShell.exec(f & " " & exe)
   do while batchExec.status = 0
      wscript.sleep 100
   loop

   ' Can delete the temporary batch file now.
   objFS.deleteFile(f)
   f=""

   if not batchExec.stdout.AtEndOfStream then
      getFullExePath = batchExec.stdout.readline 
   end if 
   set batchExec = Nothing
End Function

Function runWithOutput(cmdline)
   dim cmdExec
   'cmdLine = "devenv.com /?" ' Debug
   'WScript.echo("runWithOutput(" & cmdLine & ")") ' Debug
   ' Run the command and wait for it to finish
   set cmdExec = objShell.exec(cmdline)
   do while cmdExec.status = 0
      wscript.sleep 100
   loop

   ' Copy over it's stdout to this processes stdout
   do while not cmdExec.stdout.AtEndOfStream
      WScript.stdout.writeline(cmdExec.stdout.readline)
   loop
   ' Now copy over it's stderr to this processes stderr
   do while not cmdExec.stderr.AtEndOfStream
      WScript.stderr.writeline(cmdExec.stderr.readline)
   loop

   ' Return the error code/return code from the executed command.
   runWithOutput = cmdExec.ExitCode
   set cmdExec = Nothing
End Function


' Reference material:
' http://www.robvanderwoude.com/vbstech.html
' http://www.microsoft.com/technet/scriptcenter/resources/tales/sg1002.mspx
' http://www.microsoft.com/technet/scriptcenter/guide/sas_wsh_ywsa.mspx?mfr=true
' http://www.microsoft.com/library/media/1033/technet/images/scriptcenter/guide/sas_wsh_60c_big.gif
' http://www.microsoft.com/technet/scriptcenter/resources/qanda/jun05/hey0620.mspx
' http://www.tavaresstudios.com/Blog/post/The-last-vsvars32ps1-Ill-ever-need.aspx

' vim:ts=3:sts=3:sw=3:expandtab:cindent:
