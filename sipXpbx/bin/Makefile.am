INSTALL = @INSTALL@

bin_SCRIPTS = \
	${PACKAGE}-config \
    autodel \
    blat \
    check-fqdn

${PACKAGE}-config: $(top_builddir)/config/sipX-config
	cp $(top_builddir)/config/sipX-config ${PACKAGE}-config

SCRIPT_TEMPLATES = \
	keepalive.sh.in \
	sipx-snapshot.in \
	backup-configs.sh.in \
	backup-mailstore.sh.in \
	restore-configs.sh.in \
	restore-mailstore.sh.in

EXTRA_DIST = \
	sipxpbx.in \
  sipx-chkspace.in \
	$(bin_SCRIPTS) \
	$(SCRIPT_TEMPLATES)

edit = sed \
        -e 's,@APACHE2_HTTPD\@,@APACHE2_HTTPD@,g' \
        -e 's,@JBOSS_HOME\@,$(JBOSS_HOME),g' \
        -e 's,@prefix\@,$(prefix),g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_BINDIR\@,@SIPX_BINDIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_VXMLDATADIR\@,@SIPX_VXMLDATADIR@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@datadir\@,$(datadir),g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@VERSION\@,@VERSION@,g'

install-exec-hook : $(DESTDIR)$(sysconfdir)/init.d/sipxpbx bin-scripts \
                    $(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace

$(DESTDIR)$(sysconfdir)/init.d/sipxpbx: $(sysconfdir)/init.d sipxpbx
	$(INSTALL) -D -m 755 sipxpbx $(DESTDIR)$(sysconfdir)/init.d/sipxpbx

sipxpbx: sipxpbx.in
	$(edit) $(srcdir)/sipxpbx.in > sipxpbx

$(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace: sipx-chkspace.in
	$(edit) $(srcdir)/sipx-chkspace.in > sipx-chkspace
	$(INSTALL) -D -m 755 sipx-chkspace $(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace

$(sysconfdir)/init.d:
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/init.d 

bin-scripts: $(sysconfdir)/init.d
	for s in $(SCRIPT_TEMPLATES); \
	do \
		tgt=`basename $$s .in`; \
		$(edit) $(srcdir)/$$s > $$tgt; \
		$(INSTALL) -D -m 755 $$tgt $(DESTDIR)$(bindir)/$$tgt; \
	done

.PHONY: bin-scripts
