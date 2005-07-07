## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    config/sipX-config.in \
    CONTRIBUTORS \
    LICENSE \
    sipxportlib.spec \
    sipXportLib.dsp

SUBDIRS = bin src include syslogviewer doc

# See doc/Makefile.am why user has to explicitly type
# 'make doc'
.PHONY : doc
doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) doc

edit = sed \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g'

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxportlib.spec : sipxportlib.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxportlib.spec.in > sipxportlib.spec

# 'rpmbuild -ta' searches root of tarball for first *.spec file to build 
# RPM from
dist-hook: sipxportlib.spec
	cp sipxportlib.spec $(distdir)
