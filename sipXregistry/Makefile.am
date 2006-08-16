## Process this file with automake to produce Makefile.in
include config/project.am

INSTALL=@INSTALL@

unconditional_SUBDIRS = . include src lib bin meta etc


SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN_VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    sipxregistry.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
