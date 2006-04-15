## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak
include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = . src bin meta etc $(doc_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
	 SVN-EXPORT-VERSION \
    src/cdr \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    sipxproxy.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
