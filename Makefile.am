## Process this file with automake to produce Makefile.in

SUBDIRS = src include

pkgconfdir = @SIPXPBXCONF@

edit = sed \
        -e 's,@SIPXPBXCONF\@,$(pkgconfdir),g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g'

editscripts_IN = \
    bin/sipauthproxy.sh.in \
    bin/sipproxy.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
		target=`basename $$f .in`; \
		echo ""; echo "Installing $(bindir)/$$target"; \
		$(edit) $(srcdir)/$$f > $(bindir)/$$target; \
		chmod +x $(bindir)/$$target; \
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
		if ! test -f $(pkgconfdir)/$$target; \
		then \
			echo ""; echo "Installing default $(pkgconfdir)/$$target"; \
			$(edit) $(srcdir)/$$cfgfile > $(pkgconfdir)/$$target; \
		else \
	    	 echo ""; echo "Using existing $(pkgconfdir)/$$target"; \
	    fi; \
	done; \
	echo ""

EXTRA_DIST = \
    $(editscripts_IN) \
    $(editdata_IN) \
    sipXauthproxy.dsp \
    sipXforkingProxy.dsp \
    sipxpbx.spec.in

install-exec-hook : editscripts editdata

.PHONY : editscripts editdata

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
sipxproxy.spec : sipxproxy.spec.in
	$(edit) $(srcdir)/sipxproxy.spec.in > sipxproxy.spec

dist-hook:
	cp sipxproxy.spec $(distdir)
