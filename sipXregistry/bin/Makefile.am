
bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    $(editscripts_IN) \
    $(bin_SCRIPTS)

${PACKAGE}-config: $(top_builddir)/config/sipX-config
	cp $(top_builddir)/config/sipX-config ${PACKAGE}-config

edit = sed \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@SIPX_DBDIR\@,@SIPX_DBDIR@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@bindir\@,$(bindir),g' \
		  -e 's,@PACKAGE\@,@PACKAGE@,g' \
		  -e 's,@VERSION\@,@VERSION@,g'

editscripts_IN = \
    sipregistrar.sh.in

.PHONY : editscripts
editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
	  $(edit) $(srcdir)/$$f > $$target; \
     $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

install-exec-hook : editscripts

