LOCAL_PATH:= $(call my-dir)

# OPUS CONFIG START

include $(LOCAL_PATH)/opus/celt_sources.mk
include $(LOCAL_PATH)/opus/silk_sources.mk
include $(LOCAL_PATH)/opus/opus_sources.mk

OPUS_LT_CURRENT := 4
OPUS_LT_REVISION := 0
OPUS_LT_AGE := 4

OPUS_CFLAGS = -DHAVE_CONFIG_H
OPUS_LDFLAGS = -no-undefined -version-info $(OPUS_LT_CURRENT):$(OPUS_LT_REVISION):$(OPUS_LT_AGE)

OPUS_C_INCLUDES := $(LOCAL_PATH)/opus \
                   $(LOCAL_PATH)/opus/include \
                   $(LOCAL_PATH)/opus/celt \
                   $(LOCAL_PATH)/opus/silk 

ifndef OPUS_MATH
  OPUS_MATH := FIXED_POINT
endif
ifeq ($(OPUS_MATH),FIXED_POINT)
SILK_SOURCES += $(SILK_SOURCES_FIXED)
OPUS_C_INCLUDES += $(LOCAL_PATH)/opus/silk/fixed
else
SILK_SOURCES += $(SILK_SOURCES_FLOAT)
OPUS_C_INCLUDES += $(LOCAL_PATH)/opus/silk/float
endif

# OPUS CONFIG END

include $(CLEAR_VARS)
LOCAL_MODULE:= libopus
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES :=  $(subst celt/,opus/celt/,$(CELT_SOURCES)) \
                    $(subst silk/,opus/silk/,$(SILK_SOURCES)) \
                    $(subst src/,opus/src/,$(OPUS_SOURCES))

LOCAL_C_INCLUDES += $(OPUS_C_INCLUDES)

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += $(OPUS_CFLAGS)
LOCAL_LDFLAGS = $(OPUS_LDFLAGS)

include $(BUILD_STATIC_LIBRARY)

