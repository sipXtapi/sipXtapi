
TESTS = testsuite sandbox

INCLUDES = -I$(top_srcdir)/interface \
           -I$(top_srcdir)/sipXmediaMediaProcessing

check_PROGRAMS = testsuite sandbox

testsuite_LDADD = \
    @SIPXUNIT_LIBS@ \
    $(CPPUNIT_LIBS) \
    @SIPXTACK_LIBS@ \
    @SIPXPORT_LIBS@ \
    ../libsipXmediaProcessing.la \
    $(GIPS_OBJS)

testsuite_CXXFLAGS = -I$(SIPXUNITINC) \
                     @CPPUNIT_CFLAGS@

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@
    
# NOTE: Several media releated units tests are located in
## sipXcallLib because they made references to CallManager
testsuite_SOURCES = \
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
