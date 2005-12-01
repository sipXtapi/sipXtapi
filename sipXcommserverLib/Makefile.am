## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config/localize-sipX.sh.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak \
    config/sipXprojtop.mak \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
    SVN-EXPORT-VERSION \
    sipxcommserverlib.spec

unconditional_SUBDIRS = . src include etc meta bin

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

install-data-hook:
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(datadir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/log/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/run/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/sipxdata/tmp
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/sipxdata/sipdb

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
