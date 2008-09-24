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
    ant-targets.xml \
    ant-targets.am \
    sipxtacklib.spec \
    lib/jdom-b10.jar \
    lib/junit.jar \
    config.properties.in \
    syslog2siptrace/siptrace-merge

if SIPVIEWER
SIPVIEWER_OPT = sipviewer
endif

unconditional_SUBDIRS = . include src bin meta examples syslog2siptrace siplog2siptrace siptest $(SIPVIEWER_OPT)

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

# Get the revision number
if USE_BLDNO
BUILDPARM=--define="buildno 0.$(shell cat $(srcdir)/SVN-VERSION)"
else
BUILDPARM=--define="buildno $(RELEASE)"
endif

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(BUILDPARM) $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .
	md5sum $(PACKAGE)-$(VERSION).tar.gz >$(PACKAGE)-$(VERSION).tar.gz.md5

# RPM Spec file
sipxtacklib.spec : sipxtacklib.spec.in
	$(LocalizeSipXconfig) $(srcdir)/sipxtacklib.spec.in > sipxtacklib.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build 
# RPM from
dist-hook: sipxtacklib.spec
	cp sipxtacklib.spec $(distdir)
