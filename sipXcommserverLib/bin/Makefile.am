include $(top_builddir)/config/sipXcommon.mak

SUBDIRS = ssl-cert

dist_bin_SCRIPTS = \
	${PACKAGE}-config \
   configpp \
   get_def_addr \
   sipx-upgrade

LOCAL_SHELLSCRIPTS = \
	sipx-validate-xml

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript).in)

install-exec-hook: localize_scripts

.PHONY: localize_scripts
localize_scripts: $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript))

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript)) : $(DESTDIR)@SIPX_BINDIR@/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript)) : % : $(srcdir)/%.in
	$(LocalizeSipXconfig) $< > $@
	chmod +x $@

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

