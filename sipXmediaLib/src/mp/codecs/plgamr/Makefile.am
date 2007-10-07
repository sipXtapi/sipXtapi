## Process this file with automake to produce Makefile.in

LIBAMRNB_SUBDIR = ../../../../contrib/libamrnb
LIBAMRNB_LIB = @AMRNB_LIB_ROOT@/.libs/libamrnb.la

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRNB_INCLUDE@

lib_LTLIBRARIES = libcodec_amr.la

libcodec_amr_la_LIBADD = ${LIBAMRNB_LIB}

libcodec_amr_la_SOURCES = PlgAmr.c
libcodec_amr_la_LDFLAGS = \
	-avoid-version

libcodec_amr_la_DEPENDENCIES = ${LIBAMRNB_LIB}

${LIBAMRNB_LIB}: ${LIBAMRNB_SUBDIR}/Makefile 
	make -C @AMRNB_LIB_ROOT@

${LIBAMRNB_SUBDIR}/Makefile: ${LIBAMRNB_SUBDIR}/configure
	( cd ${LIBAMRNB_SUBDIR}; ./configure --host=${host_triplet} --build=${build_triplet} )

clean:
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C @AMRNB_LIB_ROOT@ clean; fi
distclean:
	if [ -f ${LIBAMRNB_SUBDIR}/Makefile ]; then make -C @AMRNB_LIB_ROOT@ distclean; fi
