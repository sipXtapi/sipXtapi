
SCRIPT_TEMPLATES = \
	gen-ssl-keys.sh.in \
	install-cert.sh.in	

INSTALL=@INSTALL@

EXTRA_DIST = \
	check-cert.sh \
	$(SCRIPT_TEMPLATES)

edit = sed \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@install_sh\@,@install_sh@,g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@datadir\@,$(datadir),g' \
        -e 's,@VERSION\@,@VERSION@,g'

install-exec-hook : bin-scripts

bin-scripts:
	$(INSTALL) -d -m 0755 $(DESTDIR)$(sysconfdir)/ssl 
	$(INSTALL) -D -m 0755 $(srcdir)/check-cert.sh $(DESTDIR)$(bindir)/ssl-cert/check-cert.sh
	for s in $(SCRIPT_TEMPLATES); \
	do \
		tgt=`basename $$s .in`; \
		$(edit) $(srcdir)/$$s > $$tgt; \
		$(INSTALL) -D -m 0755 $$tgt $(DESTDIR)$(bindir)/ssl-cert/$$tgt; \
	done

.PHONY: bin-scripts
