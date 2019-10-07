
if AMRNB
AMRNB_CONFIG_ARGS = --with-pic $(CONFIGURE_ARGS)
AMRNB_DIR = libamrnb
endif

if AMRWB
AMRWB_CONFIG_ARGS = --with-pic $(CONFIGURE_ARGS)
AMRWB_DIR = libamrwb
endif

# This recursion does not seem to work on all platforms.  The Makefiles
# do not seem to get created by configure.  So explicit, manual steps
# are taken below to force configure to be run in libamr*
# SUBDIRS = $(AMRNB_DIR) $(AMRWB_DIR)

if SPEEX_CONTRIB

libspeex/Makefile: libspeex/configure
	(cd libspeex; ./configure)

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

if OPUS
OPUS_CONFIG_ARGS = --with-pic $(CONFIGURE_ARGS)

# Opus targets
libopus/opus/Makefile: $(top_srcdir)/contrib/libopus/opus/configure 
	pwd
	if [ ! -d libopus/opus ]; then mkdir -p libopus/opus; fi
	(cd libopus/opus; ../../$(top_srcdir)/contrib/libopus/opus/configure $(OPUS_CONFIG_ARGS))

libopus/libogg/Makefile: $(top_srcdir)/contrib/libopus/libogg/configure 
	pwd
	if [ ! -d libopus/libogg ]; then mkdir -p libopus/libogg; fi
	(cd libopus/libogg; ../../$(top_srcdir)/contrib/libopus/libogg/configure $(OPUS_CONFIG_ARGS))

libopus/opusfile/Makefile: $(top_srcdir)/contrib/libopus/opusfile/configure 
	pwd
	if [ ! -d libopus/opusfile ]; then mkdir -p libopus/opusfile; fi
	(cd libopus/opusfile;PKG_CONFIG_PATH="../../$(top_srcdir)/contrib/libopus/libogg:../../$(top_srcdir)/contrib/libopus/opus" ../../$(top_srcdir)/contrib/libopus/opusfile/configure --disable-http $(OPUS_CONFIG_ARGS))

$(top_srcdir)/contrib/libopus/opusfile/configure $(top_srcdir)/contrib/llibopus/opusfile/missing: $(top_srcdir)/contrib/libopus/opusfile/configure.ac 
	pwd
	if [ ! -d libopus/opusfile ]; then mkdir -p libopus/opusfile; fi
	(cd libopus/opusfile; autoreconf -if)

libopus/libopusenc/Makefile: $(top_srcdir)/contrib/libopus/libopusenc/configure 
	pwd
	if [ ! -d libopus/libopusenc ]; then mkdir -p libopus/libopusenc; fi
	(cd libopus/libopusenc;PKG_CONFIG_PATH="../../$(top_srcdir)/contrib/libopus/libogg:../../$(top_srcdir)/contrib/libopus/opus" ../../$(top_srcdir)/contrib/libopus/libopusenc/configure $(OPUS_CONFIG_ARGS))

$(top_srcdir)/contrib/libopus/libopusenc/configure $(top_srcdir)/contrib/libopus/libopusenc/missing: $(top_srcdir)/contrib/libopus/libopusenc/configure.ac
	pwd
	if [ ! -d libopus/libopusenc ]; then mkdir -p libopus/libopusenc; fi
	(cd libopus/libopusenc; autoreconf -if)

libopus/opus/.libs/libopus.so libopus/opus/libopus.la: libopus/opus/Makefile
	pwd
	$(MAKE) -C libopus/opus

libopus/libogg/src/.libs/libogg.so libopus/libogg/libogg.la: libopus/libogg/Makefile
	pwd
	$(MAKE) -C libopus/libogg

libopus/opusfile/.libs/libopusfile.so libopus/opusfile/libopusfile.la: libopus/opusfile/Makefile
	pwd
	$(MAKE) -C libopus/opusfile

libopus/libopusenc/.libs/libopusenc.so libopus/libopusenc/libopusenc.la: libopus/libopusenc/Makefile
	pwd
	$(MAKE) -C libopus/libopusenc

@DEFAULT_CODECS_PATH@/libopus.so: libopus/opus/.libs/libopus.so
	cp -d libopus/opus/.libs/libopus.so* @DEFAULT_CODECS_PATH@/

@DEFAULT_CODECS_PATH@/libogg.so: libopus/libogg/src/.libs/libogg.so
	cp -d libopus/libogg/src/.libs/libogg.so* @DEFAULT_CODECS_PATH@/

@DEFAULT_CODECS_PATH@/libopusfile.so: libopus/opusfile/.libs/libopusfile.so
	cp -d libopus/opusfile/.libs/libopusfile.so* @DEFAULT_CODECS_PATH@/

@DEFAULT_CODECS_PATH@/libopusenc.so: libopus/libopusenc/.libs/libopusenc.so
	cp -d libopus/libopusenc/.libs/libopusenc.so* @DEFAULT_CODECS_PATH@/

opus: @DEFAULT_CODECS_PATH@/libopus.so @DEFAULT_CODECS_PATH@/libogg.so @DEFAULT_CODECS_PATH@/libopusfile.so @DEFAULT_CODECS_PATH@/libopusenc.so

opus_clean: libopus/opus/Makefile libopus/libogg/Makefile
	$(MAKE) -C libopus/libogg clean
	$(MAKE) -C libopus/opus clean
	$(MAKE) -C libopus/opusfile clean
	$(MAKE) -C libopus/libopusenc clean

opus_distclean: libopus/opus/Makefile libopus/libogg/Makefile
	$(MAKE) -C libopus/libogg distclean
	$(MAKE) -C libopus/opus distclean
	$(MAKE) -C libopus/opusfile distclean
	$(MAKE) -C libopus/libopusenc distclean

ALL_TARGETS := $(ALL_TARGETS) opus

CLEAN_TARGETS := $(CLEAN_TARGETS) opus_clean
endif

if AMRNB
# AMR narrow band targets
libamrnb/Makefile: $(top_srcdir)/contrib/libamrnb/configure
	pwd
	if [ ! -d libamrnb ]; then mkdir -p libamrnb; fi
	(cd libamrnb; ../$(top_srcdir)/contrib/libamrnb/configure $(AMRNB_CONFIG_ARGS))

libamrnb/.libs/libamrnb.so libamrnb/libamrnb.la: libamrnb/Makefile
	pwd
	$(MAKE) -C libamrnb

@DEFAULT_CODECS_PATH@/libamrnb.so: libamrnb/.libs/libamrnb.so
	cp -d libamrnb/.libs/libamrnb.so* @DEFAULT_CODECS_PATH@/

amrnb: @DEFAULT_CODECS_PATH@/libamrnb.so

amrnb_clean: libamrnb/Makefile
	$(MAKE) -C libamrnb clean

amrnb_distclean: libamrnb/Makefile
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
	(cd libamrwb; ../$(top_srcdir)/contrib/libamrwb/configure $(AMRWB_CONFIG_ARGS))

libamrwb/.libs/libamrwb.so libamrwb/libamrwb.la: libamrwb/Makefile
	pwd
	$(MAKE) -C libamrwb

@DEFAULT_CODECS_PATH@/libamrwb.so: libamrwb/.libs/libamrwb.so
	cp -d libamrwb/.libs/libamrwb.so* @DEFAULT_CODECS_PATH@/

amrwb: @DEFAULT_CODECS_PATH@/libamrwb.so

amrwb_clean: libamrwb/Makefile
	$(MAKE) -C libamrwb clean

amrwb_distclean: libamrwb/Makefile
	$(MAKE) -C libamrwb distclean

ALL_TARGETS := $(ALL_TARGETS) amrwb

CLEAN_TARGETS := $(CLEAN_TARGETS) amrwb_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) amrwb_distclean
endif

if G729
# libg729a targets
libg729a/Makefile: $(top_srcdir)/contrib/libg729a/configure
	pwd
	if [ ! -d libg729a ]; then mkdir -p libg729a; fi
	(cd libg729a; ../$(top_srcdir)/contrib/libg729a/configure $(AMRWB_CONFIG_ARGS))

libg729a/src/Makefile: libg729a/Makefile

libg729a/src/.libs/libg729a.so libg729a/src/libg729a.la: libg729a/Makefile libg729a/src/Makefile
	pwd
	$(MAKE) -C libg729a

@DEFAULT_CODECS_PATH@/libg729a.so: libg729a/src/.libs/libg729a.so
	cp -d libg729a/src/.libs/libg729a.so* @DEFAULT_CODECS_PATH@/

g729a: @DEFAULT_CODECS_PATH@/libg729a.so

g729a_clean: libg729a/Makefile
	$(MAKE) -C libg729a clean

g729a_distclean: libg729a/Makefile
	$(MAKE) -C libg729a distclean

ALL_TARGETS := $(ALL_TARGETS) g729a

CLEAN_TARGETS := $(CLEAN_TARGETS) g729a_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) g729a_distclean
endif

if G7221
G7221_CONFIG_ARGS = --enable-fixed-point $(CONFIGURE_ARGS)

# G.722.1 targets
libg7221/Makefile: $(top_srcdir)/contrib/libg7221/configure $(top_srcdir)/contrib/libg7221/force_fixed.txt
	pwd
	if [ ! -d libg7221 ]; then mkdir -p libg7221; fi
	(cd libg7221; ../$(top_srcdir)/contrib/libg7221/configure $(G7221_CONFIG_ARGS))

# This target is used to force the above configure script and options to be used as
# oposed to the recursive configure and automake from sipXmediaLib dir which may not
# contain the fixed point configure option.  This G.722.1 lib does not implement
# 32000 SPS G.722.1 in floating point.  So we must use fixed point on all platforms.
$(top_srcdir)/contrib/libg7221/force_fixed.txt: $(top_srcdir)/contrib/libg7221/configure
	touch $(top_srcdir)/contrib/libg7221/force_fixed.txt

libg7221/src/.libs/libg722_1.so libg7221/libg722_1.la: libg7221/Makefile
	pwd
	$(MAKE) -C libg7221

@DEFAULT_CODECS_PATH@/libg722_1.so: libg7221/src/.libs/libg722_1.so
	cp -d libg7221/src/.libs/libg722_1.so* @DEFAULT_CODECS_PATH@/

g7221: @DEFAULT_CODECS_PATH@/libg722_1.so

g7221_clean: libg7221/Makefile
	$(MAKE) -C libg7221 clean

g7221_distclean: libg7221/Makefile
	$(MAKE) -C libg7221 distclean

ALL_TARGETS := $(ALL_TARGETS) g7221

CLEAN_TARGETS := $(CLEAN_TARGETS) g7221_clean

DISTCLEAN_TARGETS := $(DISTCLEAN_TARGETS) g7221_distclean
endif

all-local: $(ALL_TARGETS)

clean-local: $(CLEAN_TARGETS)

distclean-local: $(DISTCLEAN_TARGETS)
