INCLUDES = -I$(top_srcdir)/sipXcallLib/include

SUBDIRS = tapi

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

## Full regression test and not meant for 'make check'
check_PROGRAMS = testsuite regression

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXMEDIA_LIBS@ \
    @SIPXMEDIA_MP_LIBS@ \
    @SIPXTACK_LIBS@ \
    @SIPXPORT_LIBS@ \
    @CA_LIBS@ \
    ../libsipXcall.la

testsuite_SOURCES = \
    jnibutton.cpp \
    cp/CpTestSupport.cpp \
    cp/CpTestSupport.h \
    cp/CallManagerTest.cpp 

regression_LDADD = \
    @SIPXUNIT_LIBS@ \
    @SIPXMEDIA_LIBS@ \
    @SIPXMEDIA_MP_LIBS@ \
    @SIPXTACK_LIBS@ \
    @SIPXPORT_LIBS@ \
    @CA_LIBS@ \
    ../libsipXcall.la

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

DISTCLEANFILES = Makefile.in
