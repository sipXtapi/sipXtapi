## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

EXTRA_DIST = \
    config.defs.in

INSTALL=@INSTALL@

install-data-hook :
	if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs ]; \
	then \
		echo "Using existing $(sysconfdir)/sipxpbx/config.defs"; \
	else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/config.defs"; \
		$(LocalizeSipXconfig)  $(srcdir)/config.defs.in > config.defs; \
	   $(INSTALL) -D -m 755 config.defs $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs; \
	fi
