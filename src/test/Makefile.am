
INCLUDES = -I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = sipXproxy-check

sipXproxy_check_CXXFLAGS = -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\"

check_PROGRAMS = sipXproxy-check

sipXproxy_check_LDFLAGS = -L@prefix@/lib

sipXproxy_check_LIBS = \
    libsipXcommserver.la \
    cppunit

sipXproxy_check_LDADD = ../sipforkingproxy/ForwardRules.o -lcppunit

sipXproxy_check_SOURCES = \
    unitmain.cpp \
	ForwardRulesTest.cpp

EXTRA_DATA = \
	rulesdata/simple-forwarding.xml
