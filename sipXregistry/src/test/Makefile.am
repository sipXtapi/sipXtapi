INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include 

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\"

testsuite_LDADD = \
   @SIPXUNIT_LIBS@ \
   ../libsipXregistry.la

testsuite_SOURCES = \
    RegistrarPeerTest.cpp

EXTRA_DIST = 
