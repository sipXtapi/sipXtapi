## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

unconditional_SUBDIRS = . include src lib bin meta etc

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

EXTRA_DIST = \
    config/localize-sipX.sh.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak \
    config/sipXprojtop.mak \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-EXPORT-VERSION \
    sipxregistry.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
