## Process this file with automake to produce Makefile.in

LIBAMRWB_SUBDIR = ../../../../contrib/libamrwb
LIBAMRWB_LIB = @AMRWB_LIB_ROOT@/.libs/libamrwb.la

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRWB_INCLUDE@

pkgdata_LTLIBRARIES = codec_amrwb.la

codec_amrwb_la_SOURCES = PlgAmrWB.c
codec_amrwb_la_LIBADD = ${LIBAMRWB_LIB}
codec_amrwb_la_LDFLAGS = -module -avoid-version
codec_amrwb_la_DEPENDENCIES = ${LIBAMRWB_LIB}

${LIBAMRWB_LIB}: ${LIBAMRWB_SUBDIR}/Makefile 
	make -C ${LIBAMRWB_SUBDIR}

${LIBAMRWB_SUBDIR}/Makefile: $(sipx_abs_srcdir)/contrib/libamrwb/configure
	( cd ${LIBAMRWB_SUBDIR}; $(sipx_abs_srcdir)/contrib/libamrwb/configure --host=${host_triplet} --build=${build_triplet} )

all-local: codec_amrwb.la
	echo "Installing plugin codec_amrwb.so ..."
	cp -f .libs/codec_amrwb.so @DEFAULT_CODECS_PATH@

clean-local:
	rm -f @DEFAULT_CODECS_PATH@/codec_amrwb.so
	if [ -f ${LIBAMRWB_SUBDIR}/Makefile ]; then make -C  ${LIBAMRWB_SUBDIR} clean; fi
distclean-local:
	if [ -f ${LIBAMRWB_SUBDIR}/Makefile ]; then make -C ${LIBAMRWB_SUBDIR} distclean; fi

