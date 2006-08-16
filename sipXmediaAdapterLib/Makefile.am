## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    SVN_VERSION \
    BUILDSTAMP \
    sipxmediaadapterlib.spec

unconditional_SUBDIRS = . interface sipXmediaMediaProcessing


SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
