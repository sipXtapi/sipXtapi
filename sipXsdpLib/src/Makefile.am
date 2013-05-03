## Process this file with automake to produce Makefile.in

SUBDIRS = . test

lib_LTLIBRARIES = libsipXsdp.la

INCLUDES = -I$(top_srcdir)/sipXsdpLib/include -I$(top_srcdir)/sipXportLib/include

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
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXsdp_la_LIBADD = \
	$(top_builddir)/sipXportLib/src/libsipXport.la \
        -lstdc++

# this is helpful if you're debugging something with lots of inlines (like the UtlLink code)
# 		-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

libsipXsdp_la_CXXFLAGS = \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

nodist_libsipXsdp_la_SOURCES =  \
    $(top_srcdir)/config/@PACKAGE@-buildstamp.h \
    $(top_srcdir)/config/@PACKAGE@-buildstamp.cpp

libsipXsdp_la_SOURCES =  \
    sdp/Sdp.cpp \
    sdp/SdpCandidate.cpp \
    sdp/SdpCandidatePair.cpp \
    sdp/SdpCodec.cpp \
    sdp/SdpCodecList.cpp \
    sdp/SdpDefaultCodecFactory.cpp \
    sdp/SdpMediaLine.cpp

EXTRA_DIST= \
    sdp/SdpHelperResip.cpp

