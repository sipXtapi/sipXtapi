## Process this file with automake to produce Makefile.in

LIBAMRNB_SUBDIR = ../../../../contrib/libamrnb
LIBAMRNB_LIB = @AMRNB_LIB_ROOT@/.libs/libamrnb.la

INCLUDES = -I$(top_srcdir)/sipXmediaLib/include -I$(top_srcdir)/sipXportLib/include @AMRNB_INCLUDE@

codeclib_LTLIBRARIES = codec_amr.la

codec_amr_la_SOURCES = PlgAmr.c
codec_amr_la_LIBADD = ${LIBAMRNB_LIB}
codec_amr_la_LDFLAGS = -module -avoid-version
codec_amr_la_DEPENDENCIES = ${LIBAMRNB_LIB}

${LIBAMRNB_LIB}: ${LIBAMRNB_SUBDIR}/Makefile 
	make -C @AMRNB_LIB_ROOT@

${LIBAMRNB_SUBDIR}/Makefile: ${LIBAMRNB_SUBDIR}/configure
	( cd ${LIBAMRNB_SUBDIR}; ./configure --host=${host_triplet} --build=${build_triplet} )


clean-local:
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C @AMRNB_LIB_ROOT@ clean; fi
distclean-local:
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C @AMRNB_LIB_ROOT@ distclean; fi

