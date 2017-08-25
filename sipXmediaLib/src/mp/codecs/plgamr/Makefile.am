## Process this file with automake to produce Makefile.in

LIBAMRNB_SUBDIR = ../../../../contrib/libamrnb
LIBAMRNB_LIB = @AMRNB_LIB_ROOT@/.libs/libamrnb.la

AM_CPPFLAGS  = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRNB_INCLUDE@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_amr.la

codec_amr_la_SOURCES = PlgAmr.c
codec_amr_la_LIBADD = @AMRNB_STATIC_LIB@
codec_amr_la_LDFLAGS = -module -avoid-version
codec_amr_la_DEPENDENCIES = ${LIBAMRNB_LIB}

${LIBAMRNB_LIB}: ${LIBAMRNB_SUBDIR}/Makefile 
	make -C @AMRNB_LIB_ROOT@

${LIBAMRNB_SUBDIR}/Makefile: $(sipx_abs_srcdir)/contrib/libamrnb/configure
	( cd ${LIBAMRNB_SUBDIR}; $(sipx_abs_srcdir)/contrib/libamrnb/configure --host=${host_triplet} --build=${build_triplet} )

all-local: codec_amr.la
	echo "Installing plugin codec_amr.so ..."
	cp -f .libs/codec_amr.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_amr.so
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C ${LIBAMRNB_SUBDIR} clean; fi
distclean-local:
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C ${LIBAMRNB_SUBDIR} distclean; fi

