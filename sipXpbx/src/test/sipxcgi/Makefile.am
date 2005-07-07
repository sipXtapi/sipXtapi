
INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_LDFLAGS = $(GLIB_LIBS)

testsuite_LDADD = \
    @SIPXUNIT_LDADD@ \
    ../../sipxcgi/libsipxcgi.la

testsuite_CXXFLAGS = -fno-gcse -O0 -DSRCDIR="\"$(srcdir)\"" $(GLIB_CFLAGS)

testsuite_SOURCES = \
    MockCgiInput.cpp \
    CgiValuesTest.cpp

noinst_HEADERS = \
	MockCgiInput.h

EXTRA_DIST = \
	CgiValuesTestInput.txt

