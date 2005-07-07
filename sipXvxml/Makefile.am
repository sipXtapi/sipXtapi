## Process this file with automake to produce Makefile.in

SUBDIRS = js src bin include

INSTALL = @INSTALL@

EXTRA_DIST = \
    CONTRIBUTORS \
    config/sipX-config.in \
    etc/mediaserver-config.in.in \
    sipxvxml.spec

edit = sed \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_VXMLDATADIR\@,@SIPX_VXMLDATADIR@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g'

install-data-hook :
	$(edit) $(srcdir)/etc/mediaserver-config.in.in > mediaserver-config.in ; \
	if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in ; \
	then \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/mediaserver-config.in"; \
		$(INSTALL) -D -m 644 mediaserver-config.in $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in; \
	else \
	 	echo ""; echo "Using existing $(sysconfdir)/sipxpbx/mediaserver-config.in "; \
	fi; \
	$(mkinstalldirs) $(DESTDIR)@wwwdir@/conf

# An alternative to this is to do svn export, but it's very handy to be able
# to create a distribution tarball from a working svn checkout.
dist-hook : sipxvxml.spec
	rm -rf `find $(distdir) -type d -name .svn`
	cp sipxvxml.spec $(distdir)

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxvxml.spec : $(srcdir)/sipxvxml.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxvxml.spec.in > sipxvxml.spec
