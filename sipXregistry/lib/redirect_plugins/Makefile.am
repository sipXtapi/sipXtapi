
SUBDIRS = 

lib_LTLIBRARIES = libRedirectorRegDB.la

INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libRedirectorRegDB_la_LDFLAGS = \
	-module \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libRedirectorRegDB_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@

libRedirectorRegDB_la_CXXFLAGS = \
	-I$(top_builddir)/config

libRedirectorRegDB_la_SOURCES =  \
	SipRedirectorRegDB.cpp \
	SipRedirectorRegDB.h
