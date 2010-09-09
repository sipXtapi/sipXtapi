LOCAL_PATH:= $(call my-dir)

# SPEEX CONFIG START

SPEEX_LT_CURRENT := 6
SPEEX_LT_REVISION := 0
SPEEX_LT_AGE := 5

SPEEX_CFLAGS += -DEXPORT="" -DUSE_ALLOCA
SPEEX_CFLAGS += -fstrict-aliasing -fprefetch-loop-arrays
#SPEEX_CFLAGS += -march=armv7-a -mtune=cortex-a8 -mfpu=vfpv3
#SPEEX_CFLAGS += -march=armv7-a -mtune=cortex-a8 -mfpu=neon
#SPEEX_CFLAGS += -D__ARM_ARCH_6__ -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
#SPEEX_CFLAGS += -ftree-vectorize -ffast-math -mvectorize-with-neon-quad -mno-apcs-stack-check

SPEEX_LDFLAGS = -no-undefined -version-info $(SPEEX_LT_CURRENT):$(SPEEX_LT_REVISION):$(SPEEX_LT_AGE)

ifndef SPEEX_MATH
  SPEEX_MATH := FIXED_POINT
endif
ifeq ($(SPEEX_MATH),FIXED_POINT)
  SPEEX_CFLAGS += -DFIXED_POINT -DARM5E_ASM
  ifndef SPEEX_FFT
    SPEEX_FFT := KISS_FFT
  endif
else
  SPEEX_CFLAGS += -DFLOATING_POINT
  ifndef SPEEX_FFT
    SPEEX_FFT := SMALLFT
  endif
endif

# SPEEX CONFIG END

include $(CLEAR_VARS)
LOCAL_MODULE:= libspeex
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := libspeex/cb_search.c
LOCAL_SRC_FILES += libspeex/exc_10_32_table.c
LOCAL_SRC_FILES += libspeex/exc_8_128_table.c
LOCAL_SRC_FILES += libspeex/filters.c
LOCAL_SRC_FILES += libspeex/gain_table.c
LOCAL_SRC_FILES += libspeex/hexc_table.c
LOCAL_SRC_FILES += libspeex/high_lsp_tables.c
LOCAL_SRC_FILES += libspeex/lsp.c
LOCAL_SRC_FILES += libspeex/ltp.c
LOCAL_SRC_FILES += libspeex/speex.c
LOCAL_SRC_FILES += libspeex/stereo.c
LOCAL_SRC_FILES += libspeex/vbr.c
LOCAL_SRC_FILES += libspeex/vq.c
LOCAL_SRC_FILES += libspeex/bits.c
LOCAL_SRC_FILES += libspeex/exc_10_16_table.c
LOCAL_SRC_FILES += libspeex/exc_20_32_table.c
LOCAL_SRC_FILES += libspeex/exc_5_256_table.c
LOCAL_SRC_FILES += libspeex/exc_5_64_table.c
LOCAL_SRC_FILES += libspeex/gain_table_lbr.c
LOCAL_SRC_FILES += libspeex/hexc_10_32_table.c
LOCAL_SRC_FILES += libspeex/lpc.c
LOCAL_SRC_FILES += libspeex/lsp_tables_nb.c
LOCAL_SRC_FILES += libspeex/modes.c
LOCAL_SRC_FILES += libspeex/modes_wb.c
LOCAL_SRC_FILES += libspeex/nb_celp.c
LOCAL_SRC_FILES += libspeex/quant_lsp.c
LOCAL_SRC_FILES += libspeex/sb_celp.c
LOCAL_SRC_FILES += libspeex/speex_callbacks.c
LOCAL_SRC_FILES += libspeex/speex_header.c
LOCAL_SRC_FILES += libspeex/window.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH) \
	$(SIPX_HOME)/sipXmediaLib/contrib/android

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += $(SPEEX_CFLAGS)
LOCAL_LDFLAGS = $(SPEEX_LDFLAGS)

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE:= libspeexdsp
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := libspeex/preprocess.c
LOCAL_SRC_FILES += libspeex/jitter.c
LOCAL_SRC_FILES += libspeex/mdf.c
LOCAL_SRC_FILES += libspeex/fftwrap.c
LOCAL_SRC_FILES += libspeex/filterbank.c
LOCAL_SRC_FILES += libspeex/resample.c
LOCAL_SRC_FILES += libspeex/buffer.c
LOCAL_SRC_FILES += libspeex/scal.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH) \
	$(SIPX_HOME)/sipXmediaLib/contrib/android

ifeq ($(SPEEX_FFT),KISS_FFT)
  SPEEX_CFLAGS += -DUSE_KISS_FFT
  LOCAL_SRC_FILES += libspeex/kiss_fft.c libspeex/kiss_fftr.c
else ifeq ($(SPEEX_FFT),SMALLFT)
  SPEEX_CFLAGS += -DUSE_SMALLFT
  LOCAL_SRC_FILES += libspeex/smallft.c
endif

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += $(SPEEX_CFLAGS)
LOCAL_LDFLAGS = $(SPEEX_LDFLAGS)

include $(BUILD_STATIC_LIBRARY)

