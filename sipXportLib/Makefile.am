## Process this file with automake to produce Makefile.in
include $(srcdir)/config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
    SVN_VERSION \
    sipxportlib.spec

unconditional_SUBDIRS = . bin src include syslogviewer

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES) \
	BUILDSTAMP \
	Makefile.in \
	SVN-VERSION \
	aclocal.m4 \
	autom4te.cache/output.0 \
	autom4te.cache/output.1 \
	autom4te.cache/requests \
	autom4te.cache/traces.0 \
	autom4te.cache/traces.1
