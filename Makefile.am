## Process this file with automake to produce Makefile.in

SUBDIRS = src include

INSTALL=@INSTALL@

edit = sed \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g'

editscripts_IN = \
    bin/sipauthproxy.sh.in \
    bin/sipproxy.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
		target=`basename $$f .in`; \
		echo ""; echo "Installing $(bindir)/$$target"; \
		$(edit) $(srcdir)/$$f > $$target; \
		$(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

editdata_IN = \
    etc/authproxy-config.in \
    etc/authrules.xml.in \
    etc/proxy-config.in \
    etc/forwardingrules.xml.in

editdata :
	@for cfgfile in $(editdata_IN); \
	do \
		target=`basename $$cfgfile`; \
		$(edit) $(srcdir)/$$cfgfile > $$target; \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		then \
			echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
			$(INSTALL) -D -m 644 $$target $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		else \
	    	 echo ""; echo "Using existing $(sysconfdir)/sipxpbx/$$target"; \
	    fi; \
	done; \
	echo ""

EXTRA_DIST = \
    $(editscripts_IN) \
    $(editdata_IN) \
    sipXauthproxy.dsp \
    sipXforkingProxy.dsp \
    sipxproxy.spec.in

install-exec-hook : editscripts editdata

.PHONY : editscripts editdata

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm: dist sipxproxy.spec
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
sipxproxy.spec : sipxproxy.spec.in
	$(edit) $(srcdir)/sipxproxy.spec.in > sipxproxy.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook: sipxproxy.spec
        cp sipxproxy.spec $(distdir)

