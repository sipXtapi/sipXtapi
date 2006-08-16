## Process this file with automake to produce Makefile.in
include $(srcdir)/config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    SVN_VERSION \
    BUILDSTAMP \
    sipxmedialib.spec

unconditional_SUBDIRS = . src include bin

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
