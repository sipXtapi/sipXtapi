LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libspeex
LOCAL_ARM_MODE := arm

SPEEX_LT_CURRENT := 6
SPEEX_LT_REVISION := 0
SPEEX_LT_AGE := 5

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
	$(LOCAL_PATH)

LOCAL_PRELINK_MODULE := false

#LOCAL_CFLAGS += -DFLOATING_POINT
LOCAL_CFLAGS += -DFIXED_POINT
LOCAL_CFLAGS += -DEXPORT="" -DUSE_SMALLFT
LOCAL_CFLAGS+= -O3 -fstrict-aliasing -fprefetch-loop-arrays 

LOCAL_LDFLAGS = -no-undefined -version-info $(SPEEX_LT_CURRENT):$(SPEEX_LT_REVISION):$(SPEEX_LT_AGE)

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE:= libspeexdsp
LOCAL_ARM_MODE := arm

SPEEX_LT_CURRENT := 6
SPEEX_LT_REVISION := 0
SPEEX_LT_AGE := 5

LOCAL_SRC_FILES := libspeex/preprocess.c
LOCAL_SRC_FILES += libspeex/jitter.c
LOCAL_SRC_FILES += libspeex/mdf.c
LOCAL_SRC_FILES += libspeex/fftwrap.c
LOCAL_SRC_FILES += libspeex/filterbank.c
LOCAL_SRC_FILES += libspeex/resample.c
LOCAL_SRC_FILES += libspeex/buffer.c
LOCAL_SRC_FILES += libspeex/scal.c

#if BUILD_KISS_FFT
#  FFTSRC=kiss_fft.c _kiss_fft_guts.h kiss_fft.h kiss_fftr.c kiss_fftr.h 
#elif BUILD_SMALLFT
LOCAL_SRC_FILES += libspeex/smallft.c
#endif

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)

LOCAL_PRELINK_MODULE := false

#LOCAL_CFLAGS += -DFLOATING_POINT
LOCAL_CFLAGS += -DFIXED_POINT
LOCAL_CFLAGS += -DEXPORT="" -DUSE_SMALLFT
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays 

LOCAL_LDFLAGS = -no-undefined -version-info $(SPEEX_LT_CURRENT):$(SPEEX_LT_REVISION):$(SPEEX_LT_AGE)

include $(BUILD_STATIC_LIBRARY)

