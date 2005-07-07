
INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
    $(GLIB_CFLAGS)


## Point to *.a directly so gdb can load source. 
## -static in LDFLAGS in not working as I thought it should
testsuite_LDADD = \
    @SIPXUNIT_LDADD@ \
    ../.libs/libsipXcommserver.a \
    @PCRE_LIBS@ \
    $(GLIB_LIBS)

testsuite_SOURCES = \
    UrlMappingTest.cpp

EXTRA_DIST = \
	mapdata/digits.xml \
	mapdata/escape.xml \
	mapdata/params.xml \
	mapdata/simple.xml \
	mapdata/userpat.xml \
	mapdata/vdigits.xml
