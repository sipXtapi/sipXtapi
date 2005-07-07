//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

#ifndef _SIPXTAPIINTERNAL_H /* [ */
#define _SIPXTAPIINTERNAL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "net/SipSession.h"
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SipXtapiEventDispatcher;

// STRUCTS

// TYPEDEFS
typedef struct
{
        bool bInitialized ;     /**< Is the data valid */
        bool bMuted ;           /**< Muted state (regain gain) */
        int  iGain ;            /**< Gain setting (GAIN_MIN-GAIN_MAX) */
        UtlString device;       /**< Desired auto device */
} MIC_SETTING ;

typedef struct
{
        bool bInitialized ;     /**< Is the data valid */
        int  iVol ;             /**< Gain setting (VOLUME_MIN-VOLUME_MAX) */
        UtlString device;       /**< Desired auto device */
} SPEAKER_SETTING ;

typedef struct
{
        bool bInitialized ;     /**< Is the data valid */
        bool bEnabled ;         /**< Is AEC enabled? */
} AEC_SETTING ;

typedef struct
{
    SipUserAgent*    pSipUserAgent ;
    SdpCodecFactory* pCodecFactory ;
    CallManager*     pCallManager ;
    SipLineMgr*      pLineManager ;
    SipRefreshMgr*   pRefreshManager ;
    SipXtapiEventDispatcher* mpEventDispatcher;

    MIC_SETTING      micSetting ;
    SPEAKER_SETTING  speakerSettings[2] ;
    AEC_SETTING      aecSetting ;
    SPEAKER_TYPE     enabledSpeaker ;

    char*            inputAudioDevices[MAX_AUDIO_DEVICES] ;
    char*            outputAudioDevices[MAX_AUDIO_DEVICES] ;

} SIPX_INSTANCE_DATA ;

typedef struct
{
    UtlString* callId ;
    UtlString* remoteAddress ;
    UtlString* lineURI ;
    SIPX_LINE  hLine ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
} SIPX_CALL_DATA ;


typedef struct
{
    UtlString*          strCallId ;
    SIPX_INSTANCE_DATA* pInst ;
    size_t              nCalls ;
    SIPX_CALL           hCalls[CONF_MAX_CONNECTIONS] ;
    OsRWMutex*          pMutex ;
} SIPX_CONF_DATA ;


typedef struct
{
    Url* lineURI ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
    SIPX_CONTACT_TYPE   contactType ;
} SIPX_LINE_DATA ;


typedef struct
{
        CALLBACKPROC    pCallbackProc ;
        void*           pUserData ;
    SIPX_INSTANCE_DATA* pInst ;
} LISTENER_DATA ;

typedef struct
{
    LINECALLBACKPROC    pCallbackProc;
    void*               pUserData;
    SIPX_INSTANCE_DATA* pInst;
} LINE_LISTENER_DATA;

typedef enum SIPX_LOCK_TYPE
{
    SIPX_LOCK_NONE,
    SIPX_LOCK_READ,
    SIPX_LOCK_WRITE
} SIPX_LOCK_TYPE ;

/* ============================ FUNCTION POINTER DEFINITIONS =============== */
typedef void (*sipxCallEventCallbackFn)(const void*          pSrc,
                   const char*              szCallId,
                   SipSession*          pSession,
                                   const char*          szRemoteAddress,
                                   SIPX_CALLSTATE_MAJOR eMajorState,
                                   SIPX_CALLSTATE_MINOR eMinorState);

typedef void (*sipxLineEventCallbackFn)(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINE_EVENT_TYPE_MAJOR major);

/* ============================ FUNCTIONS ================================= */
/**
 * Fire events to interested listeners.
 */
void sipxFireEvent(const void*          pSrc,
                   const char*              szCallId,
                   SipSession*          pSession,
                                   const char*          szRemoteAddress,
                                   SIPX_CALLSTATE_MAJOR eMajorState,
                                   SIPX_CALLSTATE_MINOR eMinorState) ;

/**
 * Fires a Line Event to the listeners.
 */
void sipxFireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINE_EVENT_TYPE_MAJOR major);

SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager) ;

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type);
void sipxCallReleaseLock(SIPX_CALL_DATA*, SIPX_LOCK_TYPE type);
void sipxCallObjectFree(const SIPX_CALL hCall);
SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc);
void destroyCallData(SIPX_CALL_DATA* pData);
UtlBoolean validCallData(SIPX_CALL_DATA* pData);
UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pStrCallId,
                                 UtlString* pStrRemoteAddress,
                                 UtlString* pLineId) ;
SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) ;

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type);
void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxLineObjectFree(SIPX_LINE_DATA* pData);
SIPX_LINE sipxLineLookupHandle(const char* szLineURI);
UtlBoolean validLineData(const SIPX_LINE_DATA*) ;

UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf,
                                        const SIPX_CALL hCall) ;

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type) ;
void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type) ;
void sipxConfFree(const SIPX_CONF hConf) ;
UtlBoolean validConfData(const SIPX_CONF_DATA* pData) ;

void sipxGetContactHostPort(SIPX_INSTANCE_DATA* pData,
                            SIPX_CONTACT_TYPE   contactType,
                            Url&                uri) ;
  //: Get the external host and port given the contact preference



#endif /* ] _SIPXTAPIINTERNAL_H */

#endif /* ] SIPXTAPI_EXCLUDE */
