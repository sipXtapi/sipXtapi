## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    CONTRIBUTORS \
    SVN_VERSION \
    BUILDSTAMP \
    sipxmediaadapterlib.spec

unconditional_SUBDIRS = . interface sipXmediaMediaProcessing

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
