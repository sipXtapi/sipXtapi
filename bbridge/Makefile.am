## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

unconditional_SUBDIRS = src $(doc_SUBDIRS)

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    config/localize-sipX.sh.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak \
    config/sipXprojtop.mak \
    BUILDSTAMP \
    SVN-EXPORT-VERSION \
    bbridge.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
