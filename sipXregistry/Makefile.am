## Process this file with automake to produce Makefile.in
INSTALL=@INSTALL@

SUBDIRS = src bin doc

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

editdata_IN = \
    etc/registrar-config.in \
    etc/huntgroup.xml.in \
    etc/mappingrules.xml.in \
    etc/fallbackrules.xml.in

editdata :
	@for f in $(editdata_IN); \
	do \
	  target=`basename $$f`; \
	  $(edit) $(srcdir)/$$f > $$target; \
	  tgtdir=`dirname $(sysconfdir)/sipxpbx/$$target`; \
	  if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ]; \
	  then \
		echo "Using existing $(sysconfdir)/sipxpbx/$$target"; \
	  else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
		$(INSTALL) -D -m 644 $$target $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
	  fi; \
	done

install-data-hook : editdata

EXTRA_DIST = \
    CONTRIBUTORS \
    config/sipX-config.in \
    $(editdata_IN) \
    sipXregistry.dsp \
    sipxregistry.spec

.PHONY : editdata

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	cp -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	cp -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxregistry.spec : sipxregistry.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxregistry.spec.in > sipxregistry.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
dist-hook : sipxregistry.spec
	cp sipxregistry.spec $(distdir)
