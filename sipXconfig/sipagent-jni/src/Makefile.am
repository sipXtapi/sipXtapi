## Process this file with automake to produce Makefile.in

INCLUDES = -I$(srcdir)/../include 

lib_LTLIBRARIES = libpublish-sipagent.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libpublish_sipagent_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libpublish_sipagent_la_LIBADD = \
	@SIPXTACK_LIBS@

libpublish_sipagent_la_SOURCES =  \
    sipendpoint/JNIHelpers.cpp \
    com_pingtel_pds_sds_SipConfigServerAgent.cpp
