## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    config/sipX-config.in \
    CONTRIBUTORS \
    LICENSE \
    sipXtackLib.dsp \
    ant-targets.xml \
    ant-targets.am \
    sipxtacklib.spec \
    lib/jdom-b10.jar \
    lib/junit.jar \
    config.properties.in

if SIPVIEWER
SIPVIEWER_OPT = sipviewer
endif

SUBDIRS = include src bin doc syslog2siptrace siplog2siptrace siptest $(SIPVIEWER_OPT)

# See doc/Makefile.am why user has to explicitly type
# 'make doc'
.PHONY : doc
doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) doc

edit = sed \
        -e 's,@abs_srcdir\@,@abs_srcdir@,g' \
        -e 's,@abs_builddir\@,@abs_builddir@,g' \
        -e 's,@JAVA\@,@JAVA@,g' \
        -e 's,@JAVAC_OPTIMIZED\@,@JAVAC_OPTIMIZED@,g' \
        -e 's,@JAVAC_DEBUG\@,@JAVAC_DEBUG@,g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g' \
        -e 's,@bindir\@,$(bindir),g'

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(edit) $(srcdir)/config.properties.in > $(top_builddir)/config.properties


.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxtacklib.spec : sipxtacklib.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxtacklib.spec.in > sipxtacklib.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build 
# RPM from
dist-hook: sipxtacklib.spec
	cp sipxtacklib.spec $(distdir)

