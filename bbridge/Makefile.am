## Process this file with automake to produce Makefile.in
include config/project.am

SUBDIRS = src $(doc_SUBDIRS)

RPMS = $(DEFAULT_RPMS)

EXTRA_DIST = \
    BUILDSTAMP \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    bbridge.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
