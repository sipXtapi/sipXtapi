## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

SUBDIRS = src bin

# 
# If Doxygen is enabled
if DOC
SUBDIRS += doc
endif

.PHONY : doc
doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) all

INSTALL=@INSTALL@

editdata_IN = \
    etc/authproxy-config.in \
    etc/authrules.xml.in \
    etc/proxy-config.in \
    etc/forwardingrules.xml.in

editdata :
	@for cfgfile in $(editdata_IN); \
	do \
		target=`basename $$cfgfile`; \
		$(LocalizeSipXconfig) $(srcdir)/$$cfgfile > $$target; \
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
    CONTRIBUTORS \
    config/sipX-config.in \
    $(editdata_IN) \
    sipXauthproxy.dsp \
    sipXforkingProxy.dsp \
    sipxproxy.spec

install-exec-hook : editdata

.PHONY : editdata

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist sipxproxy.spec
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxproxy.spec : sipxproxy.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(LocalizeSipXconfig) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxproxy.spec.in > sipxproxy.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook : sipxproxy.spec
	cp sipxproxy.spec $(distdir)
