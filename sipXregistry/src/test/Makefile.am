INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include 

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\"

testsuite_LDADD = \
   @SIPXUNIT_LIBS@ \
   ../libsipXregistry.la

testsuite_SOURCES = \
    RegistrarPeerTest.cpp \
    RegistrarTestCase.cpp \
    SipRegistrarServerTest.cpp

# [DEBUG ASSISTANT] List a single unit test to isolate a unit test for
# debugging or when developing a new unit test. As courtesy remove all
# unit tests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(testsuite_LDFLAGS)
sandbox_LDADD = $(testsuite_LDADD)

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    RegistrarTestCase.cpp \
    SipRegistrarServerTest.cpp

EXTRA_DIST = 
