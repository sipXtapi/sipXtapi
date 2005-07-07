## Process this file with automake to produce Makefile.in

SUBDIRS = include src bin etc

INSTALL = @INSTALL@

EXTRA_DIST = \
	 config/sipX-config.in \
	 CONTRIBUTORS LICENSE \
	 bin/httpd-sipxchange-config.sh.in \
	 sipxpbx.spec

edit = sed \
        -e 's,@JBOSS_HOME\@,$(JBOSS_HOME),g' \
        -e 's,@prefix\@,$(prefix),g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@APACHE2_MOD\@,@APACHE2_MOD@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g' \
	-e 's,@PACKAGE\@,@PACKAGE@,g' \
	-e 's,@VERSION\@,@VERSION@,g'

install-exec-hook :
	$(edit) $(srcdir)/bin/httpd-sipxchange-config.sh.in > httpd-sipxchange-config.sh
	$(INSTALL) -D -m 755 httpd-sipxchange-config.sh $(DESTDIR)$(bindir)/httpd-sipxchange-config.sh

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -v -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxpbx.spec : sipxpbx.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxpbx.spec.in > sipxpbx.spec

dist-hook : sipxpbx.spec
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib
	cp sipxpbx.spec $(distdir)
