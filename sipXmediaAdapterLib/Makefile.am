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
    sipxmediaadapterlib.spec

unconditional_SUBDIRS = . interface sipXmediaMediaProcessing

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS)
RELEASE = 2

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
sipxmediaadapterlib.spec : sipxmediaadapterlib.spec.in
	$(LocalizeSipXconfig) $(srcdir)/sipxmediaadapterlib.spec.in > sipxmediaadapterlib.spec

# 'rpmbuild -ta' searches root of tarball for first *.spec file to build 
# RPM from
dist-hook: sipxmediaadapterlib.spec
	cp sipxmediaadapterlib.spec $(distdir)
