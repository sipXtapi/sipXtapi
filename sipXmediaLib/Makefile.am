## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config/localize-sipX.sh.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak \
    config/sipXprojtop.mak \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
	 SVN-EXPORT-VERSION \
    sipxmedialib.spec

unconditional_SUBDIRS = . src include bin

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
