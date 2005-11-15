## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

unconditional_SUBDIRS = src $(doc_SUBDIRS)

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    BUILDSTAMP \
	 SVN-EXPORT-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    bbridge.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
