## Process this file with automake to produce Makefile.in

EXTRA_DIST = \
    config.defs.in

INSTALL=@INSTALL@

edit = sed \
    -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
    -e 's,@localstatedir\@,$(localstatedir),g' \
    -e 's,@bindir\@,$(bindir),g'

install-data-hook :
	if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs ]; \
	then \
		echo "Using existing $(sysconfdir)/sipxpbx/config.defs"; \
	else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/config.defs"; \
		$(edit) $(srcdir)/config.defs.in > config.defs; \
	   $(INSTALL) -D -m 755 config.defs $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs; \
	fi
