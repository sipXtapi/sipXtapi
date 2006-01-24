INCLUDES = -I$(top_srcdir)/include

SUBDIRS = testlib

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work\"

all: @abs_top_builddir@/src/test/work

@abs_top_builddir@/src/test/work:
	mkdir @abs_top_builddir@/src/test/work

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
    RegistryDbTest.cpp
