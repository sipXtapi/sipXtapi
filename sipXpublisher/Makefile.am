## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

unconditional_SUBDIRS = . src include bin etc

INSTALL = @INSTALL@

editscripts_IN = \
    bin/sipstatus.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
	 SVN-EXPORT-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    sipxpublisher.spec

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
