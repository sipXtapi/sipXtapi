// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
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

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS

// TYPEDEFS
typedef struct 
{
	bool bInitialized ;     /**< Is the data valid */
	bool bMuted ;           /**< Muted state (regain gain) */
	int  iGain ;            /**< Gain setting (GAIN_MIN-GAIN_MAX) */
} MIC_SETTING ;

typedef struct 
{
	bool bInitialized ;     /**< Is the data valid */
	int  iVol ;             /**< Gain setting (VOLUME_MIN-VOLUME_MAX) */
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
    SipRefreshMgr*	 pRefreshManager ;

    MIC_SETTING      micSetting ;
    SPEAKER_SETTING  speakerSettings[2] ;
    AEC_SETTING      aecSetting ;
    SPEAKER_TYPE	 enabledSpeaker ;

    char*             inputAudioDevices[MAX_AUDIO_DEVICES] ;
    char*             outputAudioDevices[MAX_AUDIO_DEVICES] ;

} SIPX_INSTANCE_DATA ;

typedef struct 
{
    UtlString* callId ;
    UtlString* remoteAddress ;
    UtlString* lineURI ;
    SIPX_INSTANCE_DATA* pInst ;
} SIPX_CALL_DATA ;


typedef struct 
{    
    UtlString*          strCallId ;
    SIPX_INSTANCE_DATA* pInst ;
    size_t              nCalls ;
    SIPX_CALL           hCalls[CONF_MAX_CONNECTIONS] ;
} SIPX_CONF_DATA ;

typedef struct
{
    Url* lineURI ;    
    SIPX_INSTANCE_DATA* pInst ;
} SIPX_LINE_DATA ;


typedef struct 
{
	CALLBACKPROC        pCallbackProc ;
	void*               pUserData ;
    SIPX_INSTANCE_DATA* pInst ;
} LISTENER_DATA ;

typedef struct
{
    LINECALLBACKPROC pCallbackProc;
    void*            pUserData;
    SIPX_INSTANCE_DATA* pInst;
} LINE_LISTENER_DATA;

/* ============================ FUNCTIONS ================================= */

/**
 * Fire events to interested listeners.
 */ 
void sipxFireEvent(const void*          pSrc,
                   const char*		    szCallId,
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

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall);
void sipxCallObjectFree(const SIPX_CALL hCall);
SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc);
void destroyCallData(SIPX_CALL_DATA* pData);

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine);
void sipxLineObjectFree(const SIPX_CALL hLine);
SIPX_LINE sipxLineLookupHandle(const char* szLineURI);

UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf, 
                                        const SIPX_CALL hCall) ;


#endif /* ] _SIPXTAPIINTERNAL_H */

#endif /* ] SIPXTAPI_EXCLUDE */
