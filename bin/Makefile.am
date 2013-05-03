include $(top_srcdir)/config/sipXcommon.am.inc

bin_SCRIPTS = \
	@PACKAGE@-config

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

@PACKAGE@-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > @PACKAGE@-config
	chmod +x @PACKAGE@-config

DISTCLEANFILES = \
	Makefile.in \
	@PACKAGE@-config \
	$(top_srcdir)/config/@PACKAGE@-buildstamp.cpp \
	$(top_srcdir)/config/@PACKAGE@-buildstamp.h
