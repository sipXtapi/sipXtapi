
EXTRA_DIST = \
    sipxconference.process.xml.in \
    sipxconference-config.in

install-data-hook : $(DESTDIR)$(SIPX_PROCDIR)/sipxconference.process.xml
	@for f in $(EXTRA_DIST); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
	    	echo "Using existing $(sysconfdir)/sipxpbx/$$f"; \
	    fi; \
	done
