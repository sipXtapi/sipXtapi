# 
# Copyright (C) 2009 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2009 SIPez LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
# 
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# Top level sipX Application.mk for Android
#
#

UNIT_TESTS := libsipxUnit sipxsandbox sipxportunit sipxsdpunit sipxtackunit mediasandbox sipxmediaunit sipxmediaadapterunit sipxcallunit 
SIPX_LIBS := libsipXport libsipXsdp libsipXtack libsipXmedia libsipXmediaAdapter libsipXcall libsipXtapi libpcre libsipXtapijni libsipxportjnisandbox
SIPX_CODEC_LIBS := libcodec_pcmapcmu libcodec_tones libcodec_g722

APP_MODULES := $(SIPX_LIBS) \
   $(SIPX_CODEC_LIBS) \
   $(UNIT_TESTS)

APP_PROJECT_PATH := $(call my-dir)/project
#APP_BUILD_SCRIPT := $(call my-dir)/Android.mk
#APP_OPTIM        := release
APP_OPTIM        := debug

