## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    sipxcalllib.spec

unconditional_SUBDIRS = . src examples include bin 

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

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
sipxcalllib.spec : sipxcalllib.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(LocalizeSipXconfig) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxcalllib.spec.in > sipxcalllib.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook : sipxcalllib.spec
	cp sipxcalllib.spec $(distdir)

DISTCLEANFILES = BUILDSTAMP Makefile.in aclocal.m4 autom4te.cache/* config.h.in
