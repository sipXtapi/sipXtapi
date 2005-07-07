## Process this file with automake to produce Makefile.in
SUBDIRS = ssl-cert

dist_bin_SCRIPTS = \
	${PACKAGE}-config \
    configpp \
    get_def_addr

${PACKAGE}-config: $(top_builddir)/config/sipX-config
	cp $(top_builddir)/config/sipX-config ${PACKAGE}-config

