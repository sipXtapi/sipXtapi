INCLUDES = -I$(top_srcdir)/include

TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    $(CPPUNIT_LIBS) \
    @SIPXPORT_LIBS@ \
    @SPEEXDSP_LIBS@ \
    ../libsipXmedia.la \
    $(GIPS_OBJS)

testsuite_CXXFLAGS = -I$(SIPXUNITINC) \
                     @CPPUNIT_CFLAGS@

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@
    

# The following are temporarily removed as they depend upon C++ exceptions
#    mp/MprDelayTest.cpp 
#    mp/MpWBInputOutputDeviceTest.cpp

# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
    mp/dtmf5_48khz_16b_signed.h \
    ../../../sipXportLib/src/test/os/UnitTestLogHooks.cpp \
    mp/MpAudioBufTest.cpp \
    mp/MpCodecsPerformanceTest.cpp \
    mp/MpMediaTaskTest.cpp \
    mp/MpFlowGraphTest.cpp \
    mp/MpResourceTest.cpp \
    mp/MpResourceTopologyTest.cpp \
    mp/MpTestResource.h \
    mp/MpTestResource.cpp \
    mp/MpGenericResourceTest.h \
    mp/MpGenericResourceTest.cpp \
    mp/MprBridgeTest.cpp \
    mp/MprBridgeTestWB.cpp \
    mp/MprFromFileTest.cpp \
    mp/MprFromMicTest.cpp \
    mp/MprMixerTest.cpp \
    mp/MprSpeakerSelectorTest.cpp \
    mp/MprSplitterTest.cpp \
    mp/MprToSpkrTest.cpp \
    mp/MprToneGenTest.cpp \
    mp/MpBufTest.cpp \
    mp/MpOutputManagerTest.cpp \
    mp/MpMMTimerTest.cpp 

crashesOnx86_64 = \
    mp/MpOutputDriverTest.cpp \
    ddd

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
    mp/dtmf5_48khz_16b_signed.h \
    ../../../sipXportLib/src/test/os/UnitTestLogHooks.cpp \
    mp/MpCodecsPerformanceTest.cpp

#    mp/MpOutputDriverTest.cpp 

foo = \
    mp/MprFromFileTest.cpp \
    mp/MpTestResource.cpp \
    mp/MpGenericResourceTest.cpp \
    mp/MprToneGenTest.cpp 
