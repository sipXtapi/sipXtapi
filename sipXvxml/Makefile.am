## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = . js src bin include $(doc_SUBDIRS)
RELEASE = 1

INSTALL = @INSTALL@

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    etc/mediaserver-config.in.in \
    sipxvxml.spec

install-data-hook :
	$(LocalizeSipXconfig) $(srcdir)/etc/mediaserver-config.in.in > mediaserver-config.in ; \
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
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxvxml.spec : $(srcdir)/sipxvxml.spec.in
	$(LocalizeSipXconfig) $(srcdir)/sipxvxml.spec.in > sipxvxml.spec
