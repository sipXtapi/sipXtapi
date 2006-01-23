INCLUDES = -I$(top_srcdir)/include

SUBDIRS = testlib

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work\"

testsuite_LDADD = \
	testlib/libsipXcommserverTest.la \
   ../libsipXcommserver.la

testsuite_SOURCES = \
	 RegistryDbTest.cpp \
    ResultSetRpcTest.cpp \
    UrlMappingTest.cpp

EXTRA_DIST = \
	mapdata/digits.xml \
	mapdata/escape.xml \
	mapdata/params.xml \
	mapdata/simple.xml \
	mapdata/specials.xml \
	mapdata/userpat.xml \
	mapdata/vdigits.xml \
	regdbdata/getMaxUpdate.xml
