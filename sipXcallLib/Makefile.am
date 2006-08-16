## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    SVN_VERSION \
    BUILDSTAMP \
    sipxcalllib.spec

unconditional_SUBDIRS = . src examples include bin 

include $(srcdir)/config/project.am

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES) \
	BUILDSTAMP \
	Makefile.in \
	aclocal.m4 \
	autom4te.cache/* \
	config.h.in \
	configure
