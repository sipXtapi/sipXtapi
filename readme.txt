Documentation for the sipXtapi API can be found in:
  sipXcallLib\doc\sipXtapi\html\index.html

Windows Build Hints
===================

Please see the instructions in the INSTALL doc in sipXportLib for all the required 3rd party dependencies.  Next, open sipXtapi.sln project within the sipXcallLib project.  This project will build sipXtapi, PlaceCall, ReceiveCall, and sipXezPhone.  You may need to copy the "sipXtapi[d].dll" into your working directory before you can run PlaceCall or ReceiveCall.  sipXezphone has a post-process build setup that copies those DLLs for you.

For step-by-step guide read thi page:
http://sipx-wiki.calivia.com/index.php/SipXtapi_and_sipXezPhone_Build_Environment_for_Windows

Linux Build Hints
=================
The Linux build has been tested on Fedora Core 6 and Ubuntu 6.10 (Edgy).  Automake and autoconf should do the trick for you.  If you find any missing components, you will need to install those.  See the INSTALL doc in sipXportLib for more info on these dependences.

Under FC6, I needed to yum pcre-devel and cppunit-devel.
Under Ubunutu, I need to 'apt-get install libpcre3-dev libcppunit-dev'.

In order to build with GSM and Speex codec support you will need to install libgsm (version >= 1.0.10) and libspeex (version >= 1.1) development libraries.  Their presence will be detected during 'configure' stage automatically.  If do NOT want include GSM or Speex support even if appropriate libraries are present, use "--disable-codec-gsm" and "--disable-codec-speex" switch when configuring sipXtackLib, sipXmediaLib and sipXmediaAdapterLib.

Under Ubunutu, I need to 'apt-get install libspeex-dev libgsm1-dev' to install libgsm and libspeex.

1) Build 

cd sipXportLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install
cd ..

cd sipXsdpLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install
cd ..

cd sipXtackLib
autoreconf -fi
./configure --prefix=/tmp/stage --disable-sipviewer [--disable-codec-gsm] [--disable-codec-speex]
make;make install
cd ..

cd sipXmediaLib
autoreconf -fi
./configure --prefix=/tmp/stage --enable-local-audio [--disable-codec-gsm] [--disable-codec-speex]
make;make install
cd ..

cd sipXmediaAdapterLib
autoreconf -fi
./configure --prefix=/tmp/stage [--disable-codec-gsm] [--disable-codec-speex]
make;make install
cd ..

cd sipXcallLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install
cd ..

2) Test using PlaceCall

cd examples/PlaceCall
make
./PlaceCall <IP>

PlaceCall and ReceiveCall are known to build/work with this source tree.
sipXezPhone *should* work, but hasn't been tested with the latest source.

MacOs Build Hints
=================
The MacOS build has been tested both on OS X 10.3.9, 10.4.3, and 10.4.4.  You will see some complaints about argument mismatches to the date and hostname commands during configure - ignore these.

1) Install the latest fileutils and automake/autoconf using fink.  You will also need to install cppunit 1.10.2 (did not find on fink, so we installed manually).

Here is a quick look at my versions of install, automake, and autoconf:

minimac:~/work/pax737/sipXportLib build$ which install;install --version
/sw/bin/install
install (fileutils) 4.1
Written by David MacKenzie.
...

minimac:~/work/pax737/sipXportLib build$ which autoconf;autoconf --version
/sw/bin/autoconf
autoconf (GNU Autoconf) 2.59
Written by David J. MacKenzie and Akim Demaille.
...

minimac:~/work/pax737/sipXportLib build$ which automake;automake --version
/sw/bin/automake
automake (GNU automake) 1.9.4
Written by Tom Tromey <tromey@redhat.com>.
...

2) Build the various components

cd sipXportLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

cd sipXsdpLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

cd ../sipXtackLib
autoreconf -fi
./configure --prefix=/tmp/stage --disable-sipviewer
make;make install

cd ../sipXmediaLib
autoreconf -fi
./configure --prefix=/tmp/stage --enable-local-audio
make;make install

cd ../sipXmediaAdapterLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

NOTE: for sipXmediaLib and sipXmediaAdapterLib to build you need the CoreAudio header files normally located in:
/System/Library/Frameworks/CoreAudio.framework/Versions/A/Headers
I'm not sure where these files come from - they were installed on my PowerBook, but not our minimac.  If you wish to build without these header files, remove the '--enable-local-audio' switch on the configure line.

cd ../sipXcallLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

3) Test using PlaceCall

cd examples/PlaceCall
make
./PlaceCall <IP>


PlaceCall and ReceiveCall are known to build/work with this source tree.


WinCE EVC4 Build Hints
======================
	EVC4 section written by: Keith Kyzivat - kkyzivat@sipez.com
Please see the instructions in the INSTALL doc in sipXportLib for all the
required 3rd party dependencies, and setting up EVC to find those
dependencies. Once dependencies are set, exit out of EVC, and do the
following:

* Start->Run...->"cmd"
* In the DOS shell that you'll build from, set environment variables:
	WCEROOT=<directory where EVC was installed> - you don't need
		to touch this if you've installed EVC to the default path. 
        Defaults to "C:\Program Files\Microsoft eMbedded C++ 4.0"
	SDKROOT=<directory where wince SDKs get installed> - you don't need
		to touch this if you've installed to the default path. 
        Defaults to "C:\Program Files\Windows CE Tools"
	PLATFORM=<your Windows CE target platform> - this is used to find platform
        includes, libraries, etc.
		You can figure out what this is by going to %SDKROOT%\WCE500 and
		finding your platform directory name -- that will be the value of
		PLATFORM.
		Defaults to "STANDARDSDK_500"
	CC=<compiler exe name>
	    Defaults to "cl.exe" - for ARM, you'll want to set this to "clarm.exe"
	TARGETCPU=<CPU you are targetting>
		Defaults to "emulator" - for ARM, you'll want "ARMV4I" most likely -
		other choices for ARM are "ARMV4" and "ARMV4T"

* In the dos shell run evc4_build.bat to build the libs and tests:
	C:\...\sipXtapi> evc4_build.bat

	* NOTE: Upon launching this, you'll see hundreds of messages like:
        ...\STLPORT\config\stl_apple.h(21): Could not find the file ConditionalMacros.h.
        These are expected, if annoying -- it's warning that files that aren't 
        even used (#ifdef'ed out) don't exist.

	Once the batch file completes, sipXportLib, sipXtackLib, sipXmediaLib, 
	sipXmediaAdapterLib, and all associated unit tests are built.

---
rjandreasen@gmail.com
