## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

unconditional_SUBDIRS = . src bin

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

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
		$(LocalizeSipXconfig) $(srcdir)/$$cfgfile > $$target.tmp; \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		then \
			echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
			$(INSTALL) -D -m 644 $$target.tmp $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		else \
	    	 echo ""; echo "Using existing $(sysconfdir)/sipxpbx/$$target"; \
			 case $$target in \
			   authrules.xml.in) \
				  echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/$$target"; \
				  perl -pi \
				    -e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00">|' \
				    $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				  ;; \
		    esac; \
	   fi; \
	done; \
	echo ""

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
	 SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    $(editdata_IN) \
    sipXauthproxy.dsp \
    sipXforkingProxy.dsp \
    sipxproxy.spec

install-exec-hook : editdata

.PHONY : editdata

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
# Get the revision number
if USE_BLDNO
BUILDPARM=--define="buildno 0.$(shell cat $(srcdir)/SVN-VERSION)"
else
BUILDPARM=--define="buildno $(RELEASE)"
endif

rpm : dist sipxproxy.spec
	rpmbuild -ta $(BUILDPARM) $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

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
