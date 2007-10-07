
ALL_TARGETS=
CLEAN_TARGETS=
DISTCLEAN_TARGETS=

if SPEEX_CONTRIB

.PHONY: speex_clean speex_distclean

@SPEEX_ROOT@/configure:  @SPEEX_ROOT@/configure.ac
	( cd @SPEEX_ROOT@; autoreconf -fi )

@SPEEX_ROOT@/Makefile:  @SPEEX_ROOT@/configure
	( cd @SPEEX_ROOT@; ./configure --host=${host_triplet} --build=${build_triplet} )	 

@SPEEX_ROOT@/libspeex/.libs/libspeex.la: @SPEEX_ROOT@/Makefile
	make -C @SPEEX_ROOT@

speex_clean: @SPEEX_ROOT@/Makefile
	 make -C @SPEEX_ROOT@ clean

speex_distclean:
	if test -f @SPEEX_ROOT@/Makefile; then make -C @SPEEX_ROOT@ distclean; fi


ALL_TARGETS+= @SPEEX_ROOT@/libspeex/.libs/libspeex.la
CLEAN_TARGETS+= speex_clean
DISTCLEAN_TARGETS+= speex_distclean
endif



all: ${ALL_TARGETS}

clean: ${CLEAN_TARGETS}

distclean: ${DISTCLEAN_TARGETS}
