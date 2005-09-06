INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\"

testsuite_LDADD = \
   @SIPXUNIT_LIBS@ \
   ../libsipXcommserver.la

testsuite_SOURCES = \
    UrlMappingTest.cpp

EXTRA_DIST = \
	mapdata/digits.xml \
	mapdata/escape.xml \
	mapdata/params.xml \
	mapdata/simple.xml \
	mapdata/specials.xml \
	mapdata/userpat.xml \
	mapdata/vdigits.xml
