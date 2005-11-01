
INCLUDES = -I$(top_srcdir)/src/test

lib_LTLIBRARIES = libsipXunit.la

libsipXunit_la_LDFLAGS = -static # no version info for static libs

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

libsipXunit_la_LIBADD = \
	-lcppunit

sipxincludedir = $(includedir)/sipxunit
sipxinclude_HEADERS = \
    TestUtilities.h

DISTCLEANFILES = Makefile.in
