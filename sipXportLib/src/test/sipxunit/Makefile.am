
INCLUDES = -I$(top_srcdir)/src/test

lib_LTLIBRARIES = libsipXunit.la

libsipXunit_la_LDFLAGS = -static
libsipXunit_la_SOURCES =  \
    unitmain.cpp \
    TestUtilities.cpp \
    TestRunner.cpp \
    TestRunner.h \
    TestMonitor.cpp \
    TestMonitor.h \
    TestOutputter.cpp \
    TestOutputter.h \
    TestTemplate.cpp

sipxincludedir = $(includedir)/sipxunit
sipxinclude_HEADERS = \
    TestUtilities.h


