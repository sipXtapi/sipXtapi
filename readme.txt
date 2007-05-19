Documentation for the sipXtapi API can be found in:
  sipXcallLib\doc\sipXtapi\html\index.html

Windows Build Hints
===================

Please see the instructions in the INSTALL doc in sipXportLib for all the required 3rd party dependencies.  Next, open sipXtapi.sln project within the sipXcallLib project.  This project will build sipXtapi, PlaceCall, ReceiveCall, and sipXezPhone.  You may need to copy the "sipXtapi[d].dll" into your working directory before you can run PlaceCall or ReceiveCall.  sipXezphone has a post-process build setup that copies those DLLs for you.

Linux Build Hints
=================
The Linux build has been tested on Fedora Core 5 and gentoo.  Automake and autoconf should do the trick for you.  If you find any missing components, you will need to install those.  See the INSTALL doc in sipXportLib for more info on these dependences.  Under FC5, I needed to yum pcre-devel and cppunit-devel.

1) Build 

cd sipXportLib
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
./configure --prefix=/tmp/stage --enable-local-audio
make;make install

cd ../sipXcallLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

2) Test using PlaceCall

cd examples/PlaceCall/src
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
./configure --prefix=/tmp/stage --enable-local-audio
make;make install

NOTE: for sipXmediaLib and sipXmediaAdapterLib to build you need the CoreAudio header files normally located in:
/System/Library/Frameworks/CoreAudio.framework/Versions/A/Headers
I'm not sure where these files come from - they were installed on my PowerBook, but not our minimac.  If you wish to build without these header files, remove the '--enable-local-audio' switch on the configure line.

cd ../sipXcallLib
autoreconf -fi
./configure --prefix=/tmp/stage
make;make install

3) Test using PlaceCall

cd examples/PlaceCall/src
make
./PlaceCall <IP>


PlaceCall and ReceiveCall are known to build/work with this source tree.

---
rjandreasen@gmail.com
