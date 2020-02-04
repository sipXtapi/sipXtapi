AM_CPPFLAGS = -I$(top_srcdir)/include 

TESTS = testsuite

check_PROGRAMS = testsuite sandbox

TESTDIR=$(if $(patsubst /%,,$(top_srcdir)),$(shell pwd)/$(top_srcdir),$(top_srcdir))/src/test/mp

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    $(CPPUNIT_LIBS) \
    @SIPXPORT_LIBS@ \
    @SIPXSDP_LIBS@ \
    @SIPXTACK_LIBS@ \
    @SPEEXDSP_LIBS@ \
    ../libsipXmedia.la \
    $(GIPS_OBJS) \
    @OPUS_STATIC_LIB@

# Had to add the following as AM_CPPFLAGS gets inserted in the Makefile after
# the initial declairation of CXXCOMPILE
testsuite_CXXFLAGS = -I$(SIPXUNITINC) \
		     $(AM_CPPFLAGS) \
                     -DMP_CODECS_DIR=\"$(datadir)/@PACKAGE@\" \
                     @CPPUNIT_CFLAGS@ \
                     @OPUS_INCLUDE@ \
                     @OPUS_CFLAGS@

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@
    
# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
    ../../../sipXportLib/src/test/os/UnitTestLogHooks.cpp \
    mp/MpAudioBufTest.cpp \
    mp/MpCodecsPerformanceTest.cpp \
    mp/MpCodecsQualityTest.cpp \
    mp/MpMediaTaskTest.cpp \
    mp/MpFlowGraphTest.cpp \
    mp/MpResourceTest.cpp \
    mp/MpResourceTopologyTest.cpp \
    mp/MpTestResource.cpp \
    mp/MpGenericResourceTest.h \
    mp/MpGenericResourceTest.cpp \
    mp/MprBridgeTest.cpp \
    mp/MprBridgeTestWB.cpp \
    mp/MprFromFileTest.cpp \
    mp/MprFromMicTest.cpp \
    mp/MprMixerTest.cpp \
    mp/MprNotchFilterTest.cpp \
    mp/MprRecorderTest.cpp \
    mp/MprSpeakerSelectorTest.cpp \
    mp/MprSplitterTest.cpp \
    mp/MprToSpkrTest.cpp \
    mp/MprToneDetectTest.cpp \
    mp/MprToneGenTest.cpp \
    mp/MpBufTest.cpp \
    mp/MpOutputDriverTest.cpp \
    mp/MpOutputManagerTest.cpp \
    mp/MpMMTimerTest.cpp \
    mp/MpWBInputOutputDeviceTest.cpp \
    mp/RtcpParserTest.cpp 

does_not_build = \
    mp/MprDelayTest.cpp 

codecQualitySource/%.WAV: itu_peak_recordings.rar 
	mkdir -p codecQualitySource
	mkdir tmp777; (cd tmp777; unrar e ../itu_peak_recordings.rar  CONFORMANCE\ TEST\ ITEMS)
	cp tmp777/*REF*.WAV codecQualitySource
	rm -rf tmp777

itu_peak_recordings.rar: 
	 wget https://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1387-1-200111-I\!\!ZIP-E.zip -O itu_peak_recordings.rar

mp/MpCodecsQualityTest.cpp: codecQualitySource/GREFCLA.WAV

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)

sandbox_CPPFLAGS = $(testsuite_CPPFLAGS)

sandbox_LDFLAGS = $(testsuite_LDFLAGS)

sandbox_LDADD = $(testsuite_LDADD)

sandbox_SOURCES = \
    ../../../sipXportLib/src/test/os/UnitTestLogHooks.cpp \
    mp/MpWBInputOutputDeviceTest.cpp

