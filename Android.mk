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
# Top level sipX Android.mk
#
#


SIPX_HOME := $(call my-dir)
include $(SIPX_HOME)/libpcre/Android.mk

include $(SIPX_HOME)/sipXportLib/Android.mk

include $(SIPX_HOME)/sipXsdpLib/Android.mk

include $(SIPX_HOME)/sipXtackLib/Android.mk

include $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/Android.mk

include $(SIPX_HOME)/sipXmediaLib/Android.mk

include $(SIPX_HOME)/sipXmediaAdapterLib/Android.mk

include $(SIPX_HOME)/sipXcallLib/Android.mk

include $(CLEAR_VARS)


