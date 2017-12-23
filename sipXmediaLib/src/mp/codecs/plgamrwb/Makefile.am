## Process this file with automake to produce Makefile.in

# Defining SUBDIRS makes sure the lib is up-to-date and gets installed
SUBDIRS = ../../../../contrib/libamrwb
LIBAMRWB_SUBDIR = $(SUBDIRS)
LIBAMRWB_LIB = @AMRWB_LIB_ROOT@/.libs/libamrwb.la

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXportLib/include @AMRWB_INCLUDE@

mpcodecsdir = $(datadir)/@PACKAGE@
mpcodecs_LTLIBRARIES = codec_amrwb.la

codec_amrwb_la_SOURCES = PlgAmrWB.c
codec_amrwb_la_LIBADD = @AMRWB_LIB_ROOT@/.libs/libamrwb.la
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

