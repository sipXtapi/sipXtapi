include $(top_builddir)/config/sipXcommon.mak

bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    $(editscripts_IN) \
    $(bin_SCRIPTS)

${PACKAGE}-config: $(top_builddir)/config/sipX-config
	cp $(top_builddir)/config/sipX-config ${PACKAGE}-config

editscripts_IN = \
    sipauthproxy.sh.in \
    sipproxy.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
		target=`basename $$f .in`; \
		echo ""; echo "Installing $(bindir)/$$target"; \
		$(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
		$(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

install-exec-hook: editscripts

.PHONY: editscripts

