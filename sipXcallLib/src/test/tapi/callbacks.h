//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CALLBACKS_H
#define _CALLBACKS_H

extern SIPX_CALL g_hAutoAnswerCallbackCall ;
extern SIPX_CALL g_hAutoAnswerCallbackCallOther ;
extern SIPX_LINE g_hAutoAnswerCallbackLine ;
extern SIPX_CALL g_hAutoAnswerCallbackCall2 ;
extern SIPX_CALL g_hAutoAnswerCallbackCall2Other ;
extern SIPX_LINE g_hAutoAnswerCallbackLine2 ;
extern SIPX_CALL g_hAutoAnswerHangupCallbackCall ;
extern SIPX_LINE g_hAutoAnswerHangupCallbackLine ;
extern SIPX_CALL g_hAutoRejectCallbackCall ;
extern SIPX_LINE g_hAutoRejectCallbackLine ;
extern SIPX_CALL g_hAutoRedirectCallbackCall ;
extern SIPX_LINE g_hAutoRedirectCallbackLine ;
extern SIPX_CALL g_hNewCallDetectorCall1 ;
extern SIPX_CALL g_hNewCallDetectorSourceCall1 ;
extern SIPX_CALL g_hNewCallDetectorCall2 ;
extern SIPX_CALL g_hNewCallDetectorSourceCall2 ;

void resetAutoAnswerCallback() ;

bool SIPX_CALLING_CONVENTION FlibbleTransportCallback(SIPX_TRANSPORT hTransport,
                                      const char* szDestinationIp,
                                      const int   iDestPort,
                                      const char* szLocalIp,
                                      const int   iLocalPort,
                                      const void* pData,
                                      const size_t nData,
                                      const void* pUserData);

bool SIPX_CALLING_CONVENTION AutoAnswerCallback(SIPX_EVENT_CATEGORY category, 
                        void* pInfo, 
                        void* pUserData) ;

void setAutoAnswerSecurity(SIPX_SECURITY_ATTRIBUTES* pSecurity);

bool SIPX_CALLING_CONVENTION AutoAnswerCallback_Secure(SIPX_EVENT_CATEGORY category, 
                        void* pInfo, 
                        void* pUserData) ;

void resetAutoAnswerCallback2() ;

bool SIPX_CALLING_CONVENTION AutoAnswerCallback2(SIPX_EVENT_CATEGORY category, 
                         void* pInfo, 
                         void* pUserData) ;


bool SIPX_CALLING_CONVENTION AutoAnswerHangupCallback(SIPX_EVENT_CATEGORY category, 
                              void* pInfo, 
                              void* pUserData) ;

bool SIPX_CALLING_CONVENTION AutoRejectCallback(SIPX_EVENT_CATEGORY category, 
                        void* pInfo, 
                        void* pUserData) ;

bool SIPX_CALLING_CONVENTION AutoRedirectCallback(SIPX_EVENT_CATEGORY category, 
                        void* pInfo, 
                        void* pUserData) ;

bool SIPX_CALLING_CONVENTION UniversalEventValidatorCallback(SIPX_EVENT_CATEGORY category,
                                     void* pInfo,
                                     void* pUserData) ;

bool SIPX_CALLING_CONVENTION AutoAnswerHangupRingingCallback(SIPX_EVENT_CATEGORY category, 
                                     void* pInfo, 
                                     void* pUserData);

bool SIPX_CALLING_CONVENTION NewCallDetector1(SIPX_EVENT_CATEGORY category, 
                                              void* pInfo, 
                                              void* pUserData) ;

bool SIPX_CALLING_CONVENTION NewCallDetector2(SIPX_EVENT_CATEGORY category, 
                                              void* pInfo, 
                                              void* pUserData) ;

#endif
