## Process this file with automake to produce Makefile.in
include config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    SVN_VERSION \
    BUILDSTAMP \
    sipxmedialib.spec

RPMS = $(DEFAULT_RPMS)

SUBDIRS = \
	. \
	src \
	include \
	bin \
	$(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
