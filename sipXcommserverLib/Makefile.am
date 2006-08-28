## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    SVN_VERSION \
    BUILDSTAMP \
    sipxcommserverlib.spec

SUBDIRS = \
	. \
	src \
	include \
	etc \
	meta \
	bin \
	$(doc_SUBDIRS)

install-data-hook:
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(datadir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/log/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/run/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/sipxdata/tmp
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/sipxdata/sipdb

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
