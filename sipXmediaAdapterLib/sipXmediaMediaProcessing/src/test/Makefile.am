
TESTS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/sipXmediaAdapterLib/interface \
           -I$(top_srcdir)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    $(top_builddir)/sipXportLib/src/test/sipxportunit/libsipXunit.la \
    $(CPPUNIT_LIBS) \
    $(top_builddir)/sipXtackLib/src/libsipXtack.la \
    $(top_builddir)/sipXsdpLib/src/libsipXsdp.la \
    $(top_builddir)/sipXportLib/src/libsipXport.la \
    $(top_builddir)/sipXmediaLib/src/libsipXmedia.la \
    ../libsipXmediaProcessing.la \
    $(GIPS_OBJS)

testsuite_CXXFLAGS = -I$(SIPXUNITINC) \
                     @CPPUNIT_CFLAGS@

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@
    
# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
    CpCryptoTest.cpp \
    CpPhoneMediaInterfaceTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#

sandbox_CXXFLAGS = -DSANDBOX $(testsuite_CXXFLAGS)

sandbox_CPPFLAGS = $(testsuite_CPPFLAGS)

sandbox_LDFLAGS = $(testsuite_LDFLAGS)

sandbox_LDADD = $(testsuite_LDADD)

sandbox_SOURCES = \
    CpPhoneMediaInterfaceTest.cpp
