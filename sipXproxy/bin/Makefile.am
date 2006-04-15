include $(top_builddir)/config/sipXcommon.mak

GEM_LIB_DIR=$(libdir)/ruby/gems/1.8/gems

bin_SCRIPTS = \
	${PACKAGE}-config

editscripts_IN = \
    sipauthproxy.sh \
    sipproxy.sh \
    sipxcallresolver.sh

EXTRA_DIST = \
	$(foreach file,$(editscripts_IN),$(file).in) \
   $(bin_SCRIPTS) \
  callresolver_run.in

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) \
                   $(DESTDIR)$(sysconfdir)/cron.daily/callresolver_run

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	$(LocalizeSipXconfig) -e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g' $< > $@

$(DESTDIR)$(sysconfdir)/cron.daily/callresolver_run: callresolver_run.in
	$(LocalizeSipXconfig) $(srcdir)/callresolver_run.in > callresolver_run
	$(INSTALL) -D -m 755 callresolver_run $(DESTDIR)$(sysconfdir)/cron.daily/callresolver_run

.PHONY: editscripts

