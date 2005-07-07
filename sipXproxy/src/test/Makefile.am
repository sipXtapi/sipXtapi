INCLUDES = -I$(top_srcdir)/src/sipforkingproxy

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = \
   -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
	-I$(top_srcdir)/src/cse \
	-I$(top_srcdir)/src/sipforkingproxy

## Point to *.a directly so gdb can load source.
## -static in LDFLAGS in not working as I thought it should
testsuite_LDADD = \
   @SIPXUNIT_LIBS@ \
	../cse/libcse.la

## TODO: Find a way to include ForwardingRules.o instead of
## ForwardingRules.cpp. Initial plan to do this failed because recent
## versions of libtool name the .o file "sipproxy-ForwardingRules.o" and
## early versions named it "ForwardingRules.o"
testsuite_SOURCES = \
   $(top_srcdir)/src/sipforkingproxy/ForwardRules.cpp \
   ForwardRulesTest.cpp \
	CallStateEventBuilder_XMLTest.cpp

EXTRA_DATA = \
   rulesdata/simple.xml

EXTRA_DIST = $(EXTRA_DATA)
