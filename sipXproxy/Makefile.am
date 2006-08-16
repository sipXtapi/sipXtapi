## Process this file with automake to produce Makefile.in
include config/project.am

SUBDIRS = . src bin meta etc $(doc_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN_VERSION \
    src/cdr \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    sipxproxy.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
