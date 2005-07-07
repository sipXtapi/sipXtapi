## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

unconditional_SUBDIRS = . include src bin etc sipXpark

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

INSTALL = @INSTALL@

EXTRA_DIST = \
	 config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
	 CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
	 LICENSE \
	 bin/httpd-sipxchange-config.sh.in \
	 sipxpbx.spec

install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/bin/httpd-sipxchange-config.sh.in > httpd-sipxchange-config.sh
	$(INSTALL) -D -m 755 httpd-sipxchange-config.sh $(DESTDIR)$(bindir)/httpd-sipxchange-config.sh

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
# Get the revision number
if USE_BLDNO
BUILDPARM=--define="buildno 0.$(shell cat $(srcdir)/SVN-VERSION)"
else
BUILDPARM=--define="buildno $(RELEASE)"
endif

rpm : dist
	rpmbuild -ta $(BUILDPARM) $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .
	md5sum $(PACKAGE)-$(VERSION).tar.gz >$(PACKAGE)-$(VERSION).tar.gz.md5

# RPM Spec file
sipxpbx.spec : sipxpbx.spec.in
	$(LocalizeSipXconfig) $(srcdir)/sipxpbx.spec.in > sipxpbx.spec

dist-hook : sipxpbx.spec
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib
	cp sipxpbx.spec $(distdir)
