
#ALL_TARGETS=
#CLEAN_TARGETS=
#DISTCLEAN_TARGETS=

if AMRNB
AMRNB_DIR = libamrnb
endif

if AMRWB
AMRWB_DIR = libamrwb
endif

if SPEEX_CONTRIB
SPEEXCONTRIB_DIR = libspeex
endif

SUBDIRS = $(AMRNB_DIR) $(AMRWB_DIR) $(SPEEXCONTRIB_DIR)

if SPEEX_CONTRIB

# Copy over the speex shared lib to the codec dir
@DEFAULT_CODECS_PATH@/libspeex.so: $(SPEEXCONTRIB_DIR)
	cp @SPEEX_ROOT@/libspeex/.libs/libspeex.so @DEFAULT_CODECS_PATH@/libspeex.so

# Copy over the speex shared libtool .la file to the codec dir
@DEFAULT_CODECS_PATH@/libspeex.la: $(SPEEXCONTRIB_DIR) 
	cp @SPEEX_ROOT@/libspeex/libspeex.la @DEFAULT_CODECS_PATH@/libspeex.la

endif

#all: ${ALL_TARGETS}

#clean: ${CLEAN_TARGETS}

#distclean: ${DISTCLEAN_TARGETS}
