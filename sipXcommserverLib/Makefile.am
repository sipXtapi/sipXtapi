## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    config/sipX-config.in \
    CONTRIBUTORS \
    LICENSE \
    sipxcommserverlib.spec \
    sipXcommserverLib.dsp

SUBDIRS = src include etc bin doc

edit = sed \
        -e 's,@prefix\@,$(prefix),g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g' \
		  -e 's,@PACKAGE\@,@PACKAGE@,g' \
		  -e 's,@VERSION\@,@VERSION@,g'

install-data-hook:
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(datadir)/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/log/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/run/sipxpbx
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/tmp
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/sipxdata/sipdb

# See doc/Makefile.am why user has to explicitly type
# 'make doc'
.PHONY : doc
doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) doc

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxcommserverlib.spec : sipxcommserverlib.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxcommserverlib.spec.in > sipxcommserverlib.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook : sipxcommserverlib.spec
	cp sipxcommserverlib.spec $(distdir)
