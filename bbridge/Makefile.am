## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

INSTALL=@INSTALL@

unconditional_SUBDIRS = src $(doc_SUBDIRS)

include $(srcdir)/config/project.am

SUBDIRS = $(unconditional_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    BUILDSTAMP \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    bbridge.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
