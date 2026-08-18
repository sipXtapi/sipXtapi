Documentation for the sipXtapi API can be found in:
  sipXcallLib/doc/sipXtapi/html/index.html

Android Build Instructions
=========================
see http://sipxwiki.sipez.com/wiki/SipX_Android_Build_Environment


Windows Build Hints
===================

Please see the instructions in the INSTALL doc in sipXportLib for all the required 3rd party dependencies.  Next, open sipXtapi.sln project within the sipXcallLib project.  This project will build sipXtapi, PlaceCall, ReceiveCall, and sipXezPhone.  You may need to copy the "sipXtapi[d].dll" into your working directory before you can run PlaceCall or ReceiveCall.  sipXezphone has a post-process build setup that copies those DLLs for you.

Linux Build Hints
=================
The Linux build has been tested on Fedora Core 6 and Ubuntu 6.10 (Edgy).  Automake and autoconf should do the trick for you.  If you find any missing components, you will need to install those.  See the INSTALL doc in sipXportLib for more info on these dependences.

For CentOS/Fedora/Red Hat Linux distros to setup your development environment for sipX:
  yum install subversion gcc-c++ gdb make pcre pcre-devel autoconf automake pkgconfig libtool valgrind alsa-lib alsa-lib-devel alsa-utils epel-release openssl-devel patch speex-devel

  The epel-release repo must be installed first to get access to spandsp
  yum install spandsp-devel

  If you want to build or use codecs from FFMEPG like AAC:
  yum install yasm

For Debian/Ubuntu Linux distros to setup your development environment for sipX:
  apt-get install g++ subversion gdb make libpcre3-dev autoconf automake libtool pkg-config valgrind libasound2-dev alsa-utils libssl-dev patch libspeex-dev libspandsp-dev bc

In order to build with GSM and Speex codec support you will need to install libgsm (version >= 1.0.10) and libspeex (version >= 1.1) development libraries.  Their presence will be detected during 'configure' stage automatically.  If do NOT want include GSM or Speex support even if appropriate libraries are present, use "--disable-codec-gsm" and "--disable-codec-speex" switch when configuring sipXtackLib, sipXmediaLib and sipXmediaAdapterLib.

Under Ubuntu: 'apt-get install libgsm1-dev' to install libgsm.
Under Fedora: 'yum install gsm-devel'
   note: you may need to add the linva repository: 
   'rpm -ivh http://rpm.livna.org/livna-release-6.rpm'

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
--

Makefile.gnu
There is also a makefile in the root of sipXtapi used by an automated build
system that does all steps necessary to build libraries and tests from a
freshly checked out sandbox from the subversion repository.  While not built
with normal users in mind, it can be used by them.  Find more information in
the makefile itself.




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


SRTP Support
============
SDES SRTP and DTLS SRTP are both supported in sipXmediaLib / sipXmediaAdapterLib
However this support is disabled in the default builds.

SRTP and RTCP may be enabled together.  When RTCP is enabled (the default --
i.e. EXCLUDE_RTCP is not defined), outbound RTCP reports are protected as
SRTCP and inbound SRTCP is unprotected, under the negotiated crypto suite.
Both the SDES and the DTLS-SRTP key paths are supported.

Where the SRTCP key comes from depends on the keying protocol, and the two
differ:
  - SDES (RFC 4568) carries one master key per media line, and it protects
    both SRTP and SRTCP however the ports are laid out.
  - DTLS-SRTP (RFC 5764) keys a DTLS association, and without rtcp-mux RTP
    and RTCP are separate associations with unrelated master keys.

Notes on the SRTCP implementation:
- Outbound RTCP is rendered on the RTCP manager thread rather than the media
  thread, so the RTCP renderer keeps its own libsrtp session, separate from
  MprToNet's.  MprFromNet likewise keeps separate unprotect contexts for RTP
  and RTCP.
  Under SDES both contexts hold the same master key, and that is sound because
  RFC 3711 derives the SRTP and SRTCP session keys from it with different
  labels -- an RTCP-only session is equivalent to the RTCP half of a shared
  one.  Under DTLS-SRTP without rtcp-mux they hold genuinely different master
  keys, one per association, so the split is mandatory rather than merely
  convenient.
- With DTLS-SRTP, keys do not exist until the handshake completes.  RTCP
  reports generated before that are dropped rather than sent unprotected,
  and inbound RTCP received before that is discarded.  Both mirror what the
  media path already does during the handshake window.
- DTLS-SRTP runs TWO handshakes per connection, one over the RTP socket and
  one over the RTCP socket.  RFC 5764 section 3 requires a separate DTLS
  association for each source/destination port pair, and section 4.2 has each
  association discard the key material for traffic it does not carry.  So the
  RTP association keys SRTP and the RTCP association keys SRTCP, from two
  unrelated master keys.  Both must complete before getDtlsSrtpStatus()
  reports the connection complete and verified.
- Once rtcp-mux (RFC 5761) is supported there will be a single port and hence
  a single association keying both; that work is not done yet.

The following requirements are needed to enable SRTP support in sipXtapi:
- The preprocessor define ENABLE_SRTP must be defined in:
  - sipXmediaLib                 (the SRTP/SRTCP implementation itself)
  - sipXmediaLibTest             (to compile MpSrtpTest)
  - sipXmediaAdapterLibTest      (to compile CpDtlsTest and CpSrtcpTest)
  Defining it elsewhere has no effect: no public header is conditional on it,
  and callers such as CpTopologyGraphInterface simply call into sipXmediaLib
  (see the "Will NoOp if ENABLE_SRTP is not defined" call sites).
- libsrtp (sipXmediaLib/contrib/libsrtp) must be built and linked into
  sipXmediaLib and anything that links it.
- For DTLS SRTP support, HAVE_SSL must be defined, and the OpenSSL headers and
  import libraries must be on the include and link paths, in:
  - sipXmediaLib                 (MpDtls, MpDtlsIdentity)
  - sipXmediaAdapterLibTest      (to compile CpDtlsTest)
  - sipXportLib                  (not used by DTLS SRTP itself, but enabling
                                  HAVE_SSL in sipXmediaAdapterLibTest also
                                  compiles CpCryptoTest, which links
                                  UtlCryptoKeySym and OsSocketCryptoProxy --
                                  classes wholly inside #ifdef HAVE_SSL)
  sipXtackLib also has HAVE_SSL code, but it is for SIP and HTTP over TLS and
  is unrelated to DTLS SRTP.

Application Use
---------------
SDES-SRTP Support:
SDES-SRTP is enabled on a per-connection basis through the startRtpSend() and 
startRtpReceive() overloads on CpTopologyGraphInterface that accept a 
SdpMediaLine::SdpCryptoSuiteType and a base64-encoded master key+salt string. 
The SIP/SDP layer is responsible for negotiating the crypto suite and key via 
the SDP a=crypto attribute (RFC 4568), then passing those values into the two 
startRtp* calls after setConnectionDestination(). The standard (no-crypto) 
overloads of both methods remain available for unencrypted sessions; calling 
the crypto overload on a connection already configured for DTLS-SRTP is an error.

DTLS-SRTP (RFC 5764) Support:
At application startup, optionally call the static 
CpTopologyGraphInterface::setDtlsIdentity() with paths to a PEM certificate and 
matching private key; if omitted, a self-signed ECDSA P-256 certificate is 
auto-generated on first use. Call the static getLocalDtlsFingerprint() to retrieve 
the fingerprint string to advertise in outgoing SDP a=fingerprint lines. Per 
connection, call setDtlsSrtpParams() with the remote fingerprint, hash algorithm 
(e.g. "SHA-256"), and the resolved a=setup role (TCP_SETUP_ATTRIBUTE_ACTIVE or 
TCP_SETUP_ATTRIBUTE_PASSIVE — the SIP layer must resolve actpass before calling). 
The DTLS handshake starts automatically once both setDtlsSrtpParams() and 
setConnectionDestination() have been called; SRTP keys are installed internally 
on completion. Use the non-keyed overloads of startRtpSend() and startRtpReceive() 
for DTLS-SRTP connections — do not pass keys manually. Completion or failure is 
reported via the notification dispatcher as DTLS_HANDSHAKE_COMPLETE or 
DTLS_HANDSHAKE_FAILED. The static setDtlsSrtpProfiles() and setDtlsHandshakeTimeout() 
methods allow optional process-level tuning of the offered cipher suites and 
handshake timeout respectively.