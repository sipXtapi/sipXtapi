include $(top_builddir)/config/sipXcommon.mak

SCRIPT_TEMPLATES = \
	gen-ssl-keys.sh.in \
	install-cert.sh.in	

INSTALL=@INSTALL@

EXTRA_DIST = \
	check-cert.sh \
	$(SCRIPT_TEMPLATES)

install-exec-hook : bin-scripts

bin-scripts:
	$(INSTALL) -d -m 0755 $(DESTDIR)@SIPX_CONFDIR@/ssl  
	$(INSTALL) -D -m 0755 $(srcdir)/check-cert.sh $(DESTDIR)$(bindir)/ssl-cert/check-cert.sh
	for s in $(SCRIPT_TEMPLATES); \
	do \
		tgt=`basename $$s .in`; \
		$(LocalizeSipXconfig) \
		  -e 's,@OPENSSL\@,@OPENSSL@,g' \
			$(srcdir)/$$s > $$tgt; \
		$(INSTALL) -D -m 0755 $$tgt $(DESTDIR)$(bindir)/ssl-cert/$$tgt; \
	done

.PHONY: bin-scripts
