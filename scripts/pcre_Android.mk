#
# Copyright (C) 2009 SIPez LLC.  All rights reserved.
#
#  Android makefile for libpcre
#
#  This makefile generates libpcre.so, pcregrep and pcre.h ONLY.
#  It should be amended to build libpcreposix.so, libpcrecpp.so
#  and tests.
#

ifeq ($(strip $(SIPX_HOME)),)
  LOCAL_PATH := $(call my-dir)
else
  LOCAL_PATH := $(SIPX_HOME)/libpcre
endif

###
### Build libpcre.so and pcre.h
###

include $(CLEAR_VARS)

LOCAL_MODULE := libpcre
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PRELINK_MODULE := false

intermediates := $(call local-intermediates-dir)

LOCAL_SRC_FILES :=  \
  JNI_OnLoad.c \
  pcre_compile.c \
  pcre_config.c \
  pcre_dfa_exec.c \
  pcre_exec.c \
  pcre_fullinfo.c \
  pcre_get.c \
  pcre_globals.c \
  pcre_info.c \
  pcre_internal.h \
  pcre_maketables.c \
  pcre_newline.c \
  pcre_ord2utf8.c \
  pcre_refcount.c \
  pcre_study.c \
  pcre_tables.c \
  pcre_try_flipped.c \
  pcre_ucd.c \
  pcre_valid_utf8.c \
  pcre_version.c \
  pcre_xclass.c \
  ucp.h

LOCAL_COPY_HEADERS := pcre.h

LOCAL_CFLAGS += -O3 -I. -DHAVE_CONFIG_H

GEN := $(SIPX_HOME)/libpcre/pcre_chartables.c
$(GEN): $(SIPX_HOME)/libpcre/pcre_chartables.c.dist
	$(hide) cp $(SIPX_HOME)/libpcre/pcre_chartables.c.dist $@
LOCAL_GENERATED_SOURCES += $(GEN)
LOCAL_SRC_FILES += pcre_chartables.c

GEN := $(SIPX_HOME)/libpcre/pcre.h
$(GEN): $(SIPX_HOME)/libpcre/pcre.h.generic
	$(hide) cp $(SIPX_HOME)/libpcre/pcre.h.generic $@
LOCAL_GENERATED_SOURCES += $(GEN)

GEN := $(SIPX_HOME)/libpcre/config.h
$(GEN): $(SIPX_HOME)/libpcre/config.h.generic
	$(hide) cp $(SIPX_HOME)/libpcre/config.h.generic $@
LOCAL_GENERATED_SOURCES += $(GEN)

# Hack for NDK which does not take into account LOCAL_GENERATED_SOURCES variable
$(LOCAL_PATH)/pcre_compile.c: $(SIPX_HOME)/libpcre/pcre.h $(SIPX_HOME)/libpcre/config.h

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

###
### Build pcregrep
###

include $(CLEAR_VARS)

LOCAL_MODULE := pcregrep
LOCAL_SRC_FILES := pcregrep.c
LOCAL_CFLAGS += -O3 -I. -DHAVE_CONFIG_H -DNO_RECURSE
LOCAL_SHARED_LIBRARIES := libpcre

include $(BUILD_EXECUTABLE)
