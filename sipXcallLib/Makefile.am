## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
    SVN-EXPORT-VERSION \
    sipxcalllib.spec

unconditional_SUBDIRS = . src examples include bin 

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES += \
	BUILDSTAMP \
	Makefile.in \
	aclocal.m4 \
	autom4te.cache/* \
	config.h.in \
	configure
