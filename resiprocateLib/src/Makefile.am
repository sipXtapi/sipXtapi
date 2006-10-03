## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

#SUBDIRS = . test

lib_LTLIBRARIES = libsipXresiprocateLib.la

# While the files in src are being compiled, they need to be able to see the
# Resiprocate include files, which are in what Resiprocate thinks is the
# install directory, ../build
# They also need to be able to see the include files for this glue code,
# which are in ../include
INCLUDES = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXresiprocateLib_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXresiprocateLib_la_LIBADD = \
	-lstdc++

libsipXresiprocateLib_la_CXXFLAGS = \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libsipXresiprocateLib_la_SOURCES =  \
    ../config/resiprocatelib-buildstamp.h \
    ../config/resiprocatelib-buildstamp.cpp \
	sipXexternalLogger.cpp

../config/resiprocatelib-buildstamp.cpp: ../config/resiprocatelib-buildstamp.h

../config/resiprocatelib-buildstamp.h:
	$(MAKE) -C .. BUILDSTAMP

DISTCLEANFILES = Makefile.in
