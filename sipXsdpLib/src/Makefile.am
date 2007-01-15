## Process this file with automake to produce Makefile.in

SUBDIRS = . test

lib_LTLIBRARIES = libsipXsdp.la

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

libsipXsdp_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@SSL_LDFLAGS@

libsipXsdp_la_LIBADD = \
	@SSL_LIBS@ \
	@SIPXPORT_LIBS@ \
        -lstdc++

# this is helpful if you're debugging something with lots of inlines (like the UtlLink code)
# 		-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

libsipXsdp_la_CXXFLAGS = \
	@PCRE_CFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libsipXsdp_la_SOURCES =  \
    $(top_builddir)/config/sipxsdplib-buildstamp.h \
    $(top_builddir)/config/sipxsdplib-buildstamp.cpp \
    sdp/Sdp.cpp \
    sdp/SdpMediaLine.cpp \
    sdp/SdpCodec.cpp \
    sdp/SdpCandidate.cpp \
    sdp/SdpCandidatePair.cpp

EXTRA_DIST= \
    sdp/SdpHelperResip.cpp

DISTCLEANFILES = Makefile.in
