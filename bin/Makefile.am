include $(top_builddir)/config/sipXcommon.mak

bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    $(bin_SCRIPTS)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: ../config/sipX-config $(top_builddir)/BUILDSTAMP
	BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_builddir)/config/sipX-config \
	  > ${PACKAGE}-config

