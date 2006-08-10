## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

unconditional_SUBDIRS = . include src lib bin meta etc

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN_VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    sipxregistry.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
