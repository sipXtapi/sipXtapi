INCLUDES = -I$(top_srcdir)/src/cse -I$(top_srcdir)/src/sipauthproxy -I$(top_srcdir)/src/sipforkingproxy

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite sandbox

testsuite_CXXFLAGS = \
	@CPPUNIT_CFLAGS@ \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"
	-I$(top_srcdir)/src/cse \
	-I$(top_srcdir)/src/sipauthproxy \
	-I$(top_srcdir)/src/sipforkingproxy

## Point to *.a directly so gdb can load source.
## -static in LDFLAGS is not working as I thought it should
testsuite_LDADD = \
   @SIPXUNIT_LIBS@ \
	../cse/libcse.la

## TODO: Find a way to include ForwardRules.o instead of
## ForwardRules.cpp. Initial plan to do this failed because recent
## versions of libtool name the .o file "sipproxy-ForwardRules.o" and
## early versions named it "ForwardRules.o"
testsuite_SOURCES = \
   $(top_srcdir)/src/sipforkingproxy/ForwardRules.cpp \
   ForwardRulesTest.cpp \
   CallStateEventBuilder_XMLTest.cpp \
   CallStateEventBuilder_DBTest.cpp \
   $(top_srcdir)/src/sipauthproxy/SipAaa.cpp \
   SipAaaTest.cpp \
	$(top_srcdir)/src/sipforkingproxy/SipRouter.cpp \
   SipRouterTest.cpp


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
   $(top_srcdir)/src/sipforkingproxy/ForwardRules.cpp \
   ForwardRulesTest.cpp \
   $(top_srcdir)/src/sipforkingproxy/SipRouter.cpp \
   SipRouterTest.cpp

EXTRA_DATA = \
   rulesdata/simple.xml \
   rulesdata/routing.xml


EXTRA_DIST = $(EXTRA_DATA)
