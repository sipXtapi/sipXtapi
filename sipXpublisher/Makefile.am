## Process this file with automake to produce Makefile.in

SUBDIRS = src include bin etc

INSTALL = @INSTALL@

edit = sed \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g' \
		  -e 's,@PACKAGE\@,@PACKAGE@,g' \
		  -e 's,@VERSION\@,@VERSION@,g'

editscripts_IN = \
    bin/sipstatus.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(edit) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

EXTRA_DIST = \
    CONTRIBUTORS \
    config/sipX-config.in \
    sipXpublisher.dsp \
    sipxpublisher.spec


.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxpublisher.spec : sipxpublisher.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxpublisher.spec.in > sipxpublisher.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook: sipxpublisher.spec
	cp sipxpublisher.spec $(distdir)
