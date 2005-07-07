
INCLUDES = -I$(top_srcdir)/include 

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDADD = \
    @SIPXUNIT_LDADD@ \
    ../libsipXcall.la \
    $(GLIB_LIBS)

testsuite_CXXFLAGS = $(GLIB_CFLAGS)

testsuite_SOURCES = \
    jnibutton.cpp \
    cp/CpTestSupport.cpp \
    cp/CpTestSupport.h \
    cp/CallManagerTest.cpp \
    ptapi/PtAddressForwardingTest.cpp \
    ptapi/PtAddressTest.cpp \
    ptapi/PtAudioCodecTest.cpp \
    ptapi/PtCallEventTest.cpp \
    ptapi/PtCallTest.cpp \
    ptapi/PtConnectionEventTest.cpp \
    ptapi/PtConnectionTest.cpp \
    ptapi/PtEventTest.cpp \
    ptapi/PtMediaCapabilities.cpp \
    ptapi/PtPhoneTerminalTest.cpp \
    ptapi/PtTerminalTest.cpp

## Full regression test and not meant for 'make check'
noinst_PROGRAMS = regression

regression_LDADD = \
    @SIPXUNIT_LDADD@ \
    ../libsipXcall.la \
    $(GLIB_LIBS)

regression_CXXFLAGS = $(GLIB_CFLAGS)

regression_SOURCES = \
    jnibutton.cpp \
    mp/CallManagerPlayerTest.cpp \
    mp/MpStreamPlayerTest.cpp \
    mp/MpStreamPlaylistPlayerTest.cpp \
    mp/MpStreamQueuePlayerTest.cpp \
    mp/MpTestConfig.cpp \
    mp/MyPlayerListenerHistoryKeeper.cpp \
    mp/MyPlayerListenerPoller.cpp \
    mp/MyStreamQueueHistoryKeeper.cpp
    
