## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

pkgconfdir = @SIPX_CONFDIR@

pkgconf_DATA = \
    mime.types

pbxconf_files = \
    WatchDog.xml.in \
    ProcessDefinitions.xml.in \
    resourcemap.xml.in \
    voicemail.xml.in.in \
    cbadmission.xml.in.in

wwwconf_files = \
    httpd.conf.in \
    httpd-sipxchange-common-ssl.conf.in.in \
    httpd-sipxchange-common.conf.in.in \
    httpd-sipxchange-configserver.conf.in.in \
    httpd-sipxchange-mediaserver.conf.in.in \
    httpd-sipxchange-mediaserver-ssl.conf.in.in

EXTRA_DIST = \
	$(pkgconf_DATA) \
	$(pbxconf_files) \
	$(wwwconf_files) \
	sipxchange

if DO_RPMBUILD
RPM_INSTALL=rpm-install
else
RPM_INSTALL=
endif

install-exec-hook :

install-data-hook : $(RPM_INSTALL)
	@for f in $(pbxconf_files) $(wwwconf_files); \
	do \
	  target=`basename $$f .in`; \
	  $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  tgtdir=`dirname $(sysconfdir)/sipxpbx/$$target`; \
	  if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ]; \
	  then \
		echo "Using existing $(sysconfdir)/sipxpbx/$$target"; \
	  else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
		$(INSTALL) -D -m 644 $$target $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
	  fi; \
	done

.PHONY : rpm-install
rpm-install:
	echo "RPM install of logrotate file"
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/logrotate.d
	$(INSTALL) -D -m 644 $(srcdir)/sipxchange $(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange
