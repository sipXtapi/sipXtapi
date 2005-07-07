
bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    sipXvxml.sh.in \
    voicemail_clean.sh.in \
    $(bin_SCRIPTS)

${PACKAGE}-config: $(top_builddir)/config/sipX-config
	cp $(top_builddir)/config/sipX-config ${PACKAGE}-config

edit = sed \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_VXMLDATADIR\@,@SIPX_VXMLDATADIR@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g'

install-exec-hook :
	$(edit) $(srcdir)/sipXvxml.sh.in > sipXvxml.sh
	$(INSTALL) -D -m 755 sipXvxml.sh $(DESTDIR)$(bindir)/sipXvxml.sh
	$(edit) $(srcdir)/voicemail_clean.sh.in > voicemail_clean.sh
	$(INSTALL) -D -m 755 voicemail_clean.sh $(DESTDIR)$(bindir)/voicemail_clean.sh
