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

APP_MODULES := libpcre libsipXport libsipXsdp libsipXtack libsipXmedia libsipXmediaAdater libsipXcall

APP_PROJECT_PATH := $(call my-dir)/project
#APP_BUILD_SCRIPT := $(call my-dir)/Android.mk
APP_OPTIM        := release
