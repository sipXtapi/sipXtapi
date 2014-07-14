
if AMRNB
AMRNB_DIR = libamrnb
endif

if AMRWB
AMRWB_DIR = libamrwb
endif

# This resursion does not seem to work on all platforms.  The Makefiles
# do not seem to get created by configure.  So explicit, manual steps
# are taken below to force configure to be run in libamr*
# SUBDIRS = $(AMRNB_DIR) $(AMRWB_DIR)

if SPEEX_CONTRIB

libspeex/libspeex/.libs/libspeex.so libspeex/libspeex/libspeex.la: libspeex/Makefile
	$(MAKE) -C libspeex

libspeex/libspeex/.libs/libspeexdsp.so libspeex/libspeex/libspeexdsp.la: libspeex/Makefile
	$(MAKE) -C libspeex

# Copy over the speex shared libtool .la files to the codec dir
@DEFAULT_CODECS_PATH@/libspeex.la:  libspeex/libspeex/libspeex.la 
	cp -d libspeex/libspeex/libspeex.la @DEFAULT_CODECS_PATH@/libspeex.la
@DEFAULT_CODECS_PATH@/libspeexdsp.la:  libspeex/libspeex/libspeexdsp.la 
	cp -d libspeex/libspeex/libspeexdsp.la @DEFAULT_CODECS_PATH@/libspeexdsp.la
if CROSS_COMPILING
	$(MAKE) -C libspeex install
endif

# Copy over the speex shared libs to the codec dir
@DEFAULT_CODECS_PATH@/libspeex.so: libspeex/libspeex/.libs/libspeex.so
	cp -d libspeex/libspeex/.libs/libspeex.so* @DEFAULT_CODECS_PATH@/
@DEFAULT_CODECS_PATH@/libspeexdsp.so: libspeex/libspeex/.libs/libspeexdsp.so
	cp -d libspeex/libspeex/.libs/libspeexdsp.so* @DEFAULT_CODECS_PATH@/

speex: @DEFAULT_CODECS_PATH@/libspeex.so @DEFAULT_CODECS_PATH@/libspeex.la @DEFAULT_CODECS_PATH@/libspeexdsp.so @DEFAULT_CODECS_PATH@/libspeexdsp.la

speex_clean: libspeex/Makefile
	$(MAKE) -C libspeex clean

speex_distclean: libspeex/Makefile
	$(MAKE) -C libspeex distclean

ALL_TARGETS := $(ALL_TARGETS) speex

CLEAN_TARGETS := $(CLEAN_TARGETS) speex_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) speex_distclean

.PHONY: speex speex_clean speex_distclean opus opus_clean opus_distclean

else

.PHONY: opus opus_clean opus_distclean

endif

# Opus targets
libopus/opus/Makefile: $(top_srcdir)/contrib/libopus/opus/configure
	pwd
	if [ ! -d libopus/opus ]; then mkdir -p libopus/opus; fi
	(cd libopus/opus; $(top_srcdir)/../../contrib/libopus/opus/configure $(OPUS_CONFIG_OPTS))

libopus/opus/.libs/libopus.so libopus/opus/libopus.la: libopus/opus/Makefile
	pwd
	$(MAKE) -C libopus/opus

@DEFAULT_CODECS_PATH@/libopus.so: libopus/opus/.libs/libopus.so
	cp -d libopus/opus/.libs/libopus.so* @DEFAULT_CODECS_PATH@/

opus: @DEFAULT_CODECS_PATH@/libopus.so

opus_clean: libopus/opus/Makefile
	$(MAKE) -C libopus/opus clean

opus_distclean: libopus/opus/Makefile
	$(MAKE) -C libopus/opus distclean

ALL_TARGETS := $(ALL_TARGETS) opus

CLEAN_TARGETS := $(CLEAN_TARGETS) opus_clean

if AMRNB
# AMR narrow band targets
libamrnb/Makefile: $(top_srcdir)/contrib/libamrnb/configure
	pwd
	if [ ! -d libamrnb ]; then mkdir -p libamrnb; fi
	(cd libamrnb; $(top_srcdir)/../../contrib/libamrnb/configure)

libamrnb/.libs/libamrnb.so libamrnb/libamrnb.la: libamrnb/Makefile
	pwd
	$(MAKE) -C libamrnb

@DEFAULT_CODECS_PATH@/libamrnb.so: libamrnb/.libs/libamrnb.so
	cp -d libamrnb/.libs/libamrnb.so* @DEFAULT_CODECS_PATH@/

amrnb: @DEFAULT_CODECS_PATH@/libamrnb.so

amrnb_clean: libamrnb/Makefile
	$(MAKE) -C libamrnb clean

amrnb: libamrnb/Makefile
	$(MAKE) -C libamrnb distclean

ALL_TARGETS := $(ALL_TARGETS) amrnb

CLEAN_TARGETS := $(CLEAN_TARGETS) amrnb_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) amrnb_distclean
endif

if AMRWB
# AMR wide band targets
libamrwb/Makefile: $(top_srcdir)/contrib/libamrwb/configure
	pwd
	if [ ! -d libamrwb ]; then mkdir -p libamrwb; fi
	(cd libamrwb; $(top_srcdir)/../../contrib/libamrwb/configure)

libamrwb/.libs/libamrwb.so libamrwb/libamrwb.la: libamrwb/Makefile
	pwd
	$(MAKE) -C libamrwb

@DEFAULT_CODECS_PATH@/libamrwb.so: libamrwb/.libs/libamrwb.so
	cp -d libamrwb/.libs/libamrwb.so* @DEFAULT_CODECS_PATH@/

amrwb: @DEFAULT_CODECS_PATH@/libamrwb.so

amrwb_clean: libamrwb/Makefile
	$(MAKE) -C libamrwb clean

amrwb: libamrwb/Makefile
	$(MAKE) -C libamrwb distclean

ALL_TARGETS := $(ALL_TARGETS) amrwb

CLEAN_TARGETS := $(CLEAN_TARGETS) amrwb_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) amrwb_distclean
endif


all-local: $(ALL_TARGETS)

clean-local: $(CLEAN_TARGETS)

distclean-local: $(DISTCLEAN_TARGETS)
