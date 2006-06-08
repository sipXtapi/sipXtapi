//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SIPXTAPIINTERNAL_H /* [ */
#define _SIPXTAPIINTERNAL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "tapi/SipXMessageObserver.h"
#ifdef VOICE_ENGINE
#    include "tapi/GipsDefs.h"
#endif
#include "net/SipSession.h"
#include "net/SipUserAgent.h"
#include "net/SipSubscribeClient.h"
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"
#include "os/OsMutex.h"

// DEFINES
/** sipXtapi can be configured to expire after a certain date */
//#define SIPXTAPI_EVAL_EXPIRATION 

#ifdef SIPXTAPI_EVAL_EXPIRATION
#  define EVAL_EXPIRE_MONTH     0       // zero based
#  define EVAL_EXPIRE_DAY       31
#  define EVAL_EXPIRE_YEAR      2006
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SipSubscribeServer;
class SipSubscribeClient;
class CallManager ;
class SipUserAgent ;
class SipRefreshMgr ;

// STRUCTS

// TYPEDEFS
typedef struct MIC_SETTING
{
        bool bInitialized ;     /**< Is the data valid */
        bool bMuted ;           /**< Muted state (regain gain) */
        int  iGain ;            /**< Gain setting (GAIN_MIN-GAIN_MAX) */
        UtlString device;       /**< Desired auto device */
} MIC_SETTING ;

typedef struct SPEAKER_SETTING
{
        bool bInitialized ;     /**< Is the data valid */
        int  iVol ;             /**< Gain setting (VOLUME_MIN-VOLUME_MAX) */
        UtlString device;       /**< Desired auto device */
} SPEAKER_SETTING ;

typedef struct AEC_SETTING
{
        bool bInitialized ;     /**< Is the data valid */
        SIPX_AEC_MODE mode ;    /**< Is AEC enabled? */
} AEC_SETTING ;

typedef struct AGC_SETTING
{
        bool bInitialized ;     /**< Is the data valid */
        bool bEnabled;          /**< Is AGC enabled? */
} AGC_SETTING ;


typedef struct NOISE_REDUCTION_SETTING
{
        bool bInitialized ;     /**< Is the data valid */
        SIPX_NOISE_REDUCTION_MODE mode ;    /**< Is NR enabled? */
} NOISE_REDUCTION_SETTING ;


typedef struct AUDIO_CODEC_PREFERENCES
{
    bool              bInitialized;  /**< Is the data valid */
    int               numCodecs;     /**< Number of codecs */
    SIPX_AUDIO_BANDWIDTH_ID codecPref; /**< Numeric Id of codec preference */
    SIPX_AUDIO_BANDWIDTH_ID fallBack;  /**< Fallback id if codec setting fails */
    UtlString         sPreferences;  /**< List of preferred codecs */
    SdpCodec**        sdpCodecArray; /**< Pointer to an array of codecs */
} AUDIO_CODEC_PREFERENCES;

typedef struct VIDEO_CODEC_PREFERENCES
{
    bool              bInitialized;    /**< Is the data valid */
    int               numCodecs;       /**< Number of codecs */
    SIPX_VIDEO_BANDWIDTH_ID codecPref; /**< Numeric Id of codec preference */
    SIPX_VIDEO_BANDWIDTH_ID fallBack;  /**< Fallback id if codec setting fails */
    UtlString         sPreferences;    /**< List of preferred codecs */
    SdpCodec**        sdpCodecArray;   /**< Pointer to an array of codecs */
} VIDEO_CODEC_PREFERENCES;

typedef struct TONE_STATES
{
    bool             bInitialized;
    bool             tonePlaying;
} TONE_STATES;

typedef struct SIPX_INSTANCE_DATA
{
    SipUserAgent*    pSipUserAgent ;
    SdpCodecFactory* pCodecFactory ;
    CallManager*     pCallManager ;
    SipLineMgr*      pLineManager ;
    SipRefreshMgr*   pRefreshManager ;
    SipSubscribeServer* pSubscribeServer;
    SipSubscribeClient* pSubscribeClient;
    SipRefreshManager* pSipRefreshManager ;
    SipDialogMgr* pDialogManager ;

    MIC_SETTING      micSetting ;
    SPEAKER_SETTING  speakerSettings[2] ;
    AEC_SETTING      aecSetting ;
    AGC_SETTING      agcSetting ;
    NOISE_REDUCTION_SETTING nrSetting ;
    SPEAKER_TYPE     enabledSpeaker ;
    AUDIO_CODEC_PREFERENCES 
                     audioCodecSetting;
    VIDEO_CODEC_PREFERENCES 
                     videoCodecSetting;
    TONE_STATES      toneStates;



    char*            inputAudioDevices[MAX_AUDIO_DEVICES] ;
    char*            outputAudioDevices[MAX_AUDIO_DEVICES] ;
    SipXMessageObserver* pMessageObserver;
    OsNotification   *pStunNotification ;   /**< Signals the initial stun success/failure
                                                 when calling sipXconfigEnableStun */
    OsMutex*         pLock ;
    int              nCalls ;       /**< Counter for inprocess calls */
    int              nConferences ; /**< Counter for inprocess conferences */
    int              nLines ;       /**< Counter for inprocess lines */
    void*            pVoiceEngine;  /**< Cache VoiceEngine pointer */
    char            dbLocation[256];    /**< Cache cert db location > */
    char            myCertNickname[32]; /**< Cache certificate nickname > */
    char            dbPassword[32];    /**< Cache cert db password > */
    bool             bShortNames;   /**< short names in sip messages >*/
    bool             bAllowHeader;  /**< use allow header in sip messages>*/
    bool             bDateHeader;   /**< use Date header in sip messages>*/
    char             szAcceptLanguage[16]; /**< accept language to use in sip messages>*/
    char             szLocationHeader[256]; /**< location header */
    bool             bRtpOverTcp;   /**< allow RTP over TCP */
} SIPX_INSTANCE_DATA ;

typedef enum SIPX_INTERNAL_CALLSTATE
{
    SIPX_INTERNAL_CALLSTATE_UNKNOWN = 0,        /** Unknown call state */
    SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT,   /** Early dialog: outbound */
    SIPX_INTERNAL_CALLSTATE_INBOUND_ATEMPT,     /** Early dialog: inbound */
    SIPX_INTERNAL_CALLSTATE_CONNECTED,          /** Active call - remote audio */
    SIPX_INTERNAL_CALLSTATE_HELD,               /** both on hold due to a local hold */
    SIPX_INTERNAL_CALLSTATE_REMOTE_HELD,        /** Remotely held call */
    SIPX_INTERNAL_CALLSTATE_BRIDGED,            /** Locally held call, bridging */
    SIPX_INTERNAL_CALLSTATE_DISCONNECTED,       /** Disconnected or failed */
    SIPX_INTERNAL_CALLSTATE_DESTROYING,         /** In the process of being destroyed */
} SIPX_INTERNAL_CALLSTATE ;


typedef struct SIPX_CALL_DATA
{
    UtlString* callId;
    UtlString* sessionCallId;
    UtlString* ghostCallId;    
    UtlString* remoteAddress ;
    UtlString* lineURI ;
    UtlString* contactAddress ;
    SIPX_LINE  hLine ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex* pMutex ;
    SIPX_CONF hConf ;
    SIPX_SECURITY_ATTRIBUTES security;
    SIPX_VIDEO_DISPLAY display;
    UtlBoolean bRemoveInsteadOfDrop ;   /** Remove the call instead of dropping it 
                                            -- this is used as part of consultative 
                                            transfer when we are the transfer target 
                                            and need to replace a call leg within 
                                            the same CpPeerCall. */
    SIPX_CALLSTATE_EVENT lastCallstateEvent ;
    SIPX_CALLSTATE_CAUSE lastCallstateCause ;

    SIPX_MEDIA_EVENT lastLocalMediaAudioEvent ;
    SIPX_MEDIA_EVENT lastLocalMediaVideoEvent ;
    SIPX_MEDIA_EVENT lastRemoteMediaAudioEvent ;
    SIPX_MEDIA_EVENT lastRemoteMediaVideoEvent ;

    SIPX_INTERNAL_CALLSTATE state ;
    UtlBoolean bInFocus ;
    int connectionId;                  /** Cache the connection id */
    SIPX_TRANSPORT hTransport;
    bool bHoldAfterConnect;            /** Used if we are the transfer target, and the
                                           replaced call is HELD or REMOTE_HELD, then
                                           this flag is set, and indicates that the call
                                           should be placed on hold after the connection
                                           is established. */
    bool bCallHoldInvoked;             /** Set to true if sipxCallHold has been invoked.
                                           Set to fales if sipxCallUnhold has been invoked. */                                          
    bool bTonePlaying;
    int nFilesPlaying;
} SIPX_CALL_DATA ;

typedef enum CONF_HOLD_STATE
{
    CONF_STATE_UNHELD = 0,
    CONF_STATE_BRIDGING_HOLD,
    CONF_STATE_NON_BRIDGING_HOLD,
} CONF_HOLD_STATE;

typedef struct
{
    UtlString*          strCallId ;
    SIPX_INSTANCE_DATA* pInst ;
    size_t              nCalls ;
    SIPX_CALL           hCalls[CONF_MAX_CONNECTIONS] ;
    CONF_HOLD_STATE     confHoldState;
    SIPX_TRANSPORT hTransport;
    int                 nNumFilesPlaying;
    OsRWMutex*          pMutex ;
} SIPX_CONF_DATA ;

typedef struct
{
    Url* lineURI ;
    SIPX_INSTANCE_DATA* pInst ;
    OsRWMutex*          pMutex ;
    SIPX_CONTACT_TYPE   contactType ;
    UtlSList*           pLineAliases ;
} SIPX_LINE_DATA ;

typedef struct
{
    SIPX_INFO_INFO infoData;
    SIPX_INSTANCE_DATA* pInst;
    SipSession* pSession;
    OsRWMutex*          pMutex;
} SIPX_INFO_DATA;

typedef struct
{
    SIPX_INSTANCE_DATA* pInst;
    UtlString* pResourceId;
    UtlString* pEventType;
    HttpBody* pContent;
    OsRWMutex* pMutex;
} SIPX_PUBLISH_DATA;

typedef struct
{
    SIPX_INSTANCE_DATA* pInst;
    UtlString* pDialogHandle;
    OsRWMutex* pMutex;
} SIPX_SUBSCRIPTION_DATA;

#define MAX_TRANSPORT_NAME 32
class SIPX_TRANSPORT_DATA
{
public:
    SIPX_TRANSPORT_DATA() :
        pInst(NULL),
        bIsReliable(false),
        iLocalPort(-1),
        pFnWriteProc(NULL),
        pMutex(NULL),
        hTransport(0),
        pUserData(NULL)
    {
        memset(szLocalIp, 0, sizeof(szLocalIp));
        memset(szTransport, 0, sizeof(szTransport));
        memset(cRoutingId, 0, sizeof(cRoutingId)) ;
    }
    /** Copy constructor. */
    SIPX_TRANSPORT_DATA(const SIPX_TRANSPORT_DATA& ref)
    {
        copy(ref);
    }
    /** Assignment operator. */
    SIPX_TRANSPORT_DATA& operator=(const SIPX_TRANSPORT_DATA& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;
        
        return copy(ref);
    }    
    
    SIPX_TRANSPORT_DATA& copy(const SIPX_TRANSPORT_DATA& ref)
    {
        hTransport = ref.hTransport;
        pInst = ref.pInst;
        bIsReliable = ref.bIsReliable;
        memset(szTransport, 0, sizeof(szTransport)) ;
        strncpy(szTransport, ref.szTransport, MAX_TRANSPORT_NAME - 1);
        memset(szLocalIp, 0, sizeof(szLocalIp)) ;
        strncpy(szLocalIp, ref.szLocalIp, sizeof(szLocalIp)-1);
        memset(cRoutingId, 0, sizeof(cRoutingId)) ;
        strncpy(cRoutingId, ref.cRoutingId, sizeof(cRoutingId)-1);
        iLocalPort = ref.iLocalPort;
        pFnWriteProc = ref.pFnWriteProc;
        pUserData = ref.pUserData ;        
        return *this;
    }

    static const bool isCustomTransport(const SIPX_TRANSPORT_DATA* const pTransport)
    {
        bool bRet = false;
        if (pTransport)
        {
            if (strlen(pTransport->szTransport) > 0)
            {
                bRet = true;
            }
        }
        return bRet;
    }
    
    SIPX_TRANSPORT            hTransport;
    SIPX_INSTANCE_DATA*       pInst;
    bool                      bIsReliable;
    char                      szTransport[MAX_TRANSPORT_NAME];
    char                      szLocalIp[32];
    int                       iLocalPort;
    SIPX_TRANSPORT_WRITE_PROC pFnWriteProc;
    OsRWMutex*                pMutex;
    const void*               pUserData;
    char                      cRoutingId[64] ;
} ;

/**
 * internal sipXtapi structure that binds a
 * an event callback proc
 * with an instance pointer and user data
 */
typedef struct
{
    SIPX_EVENT_CALLBACK_PROC pCallbackProc;
    void* pUserData;
    SIPX_INSTANCE_DATA* pInst;
} EVENT_LISTENER_DATA;

typedef enum SIPX_LOCK_TYPE
{
    SIPX_LOCK_NONE,
    SIPX_LOCK_READ,
    SIPX_LOCK_WRITE
} SIPX_LOCK_TYPE ;


/* ============================ FUNCTION POINTER DEFINITIONS =============== */

typedef void (*sipxCallEventCallbackFn)(const void* pSrc,
                                        const char* szCallId,
                                        SipSession* pSession,
                                        const char* szRemoteAddress,
                                        SIPX_CALLSTATE_EVENT event,
                                        SIPX_CALLSTATE_CAUSE cause,
                                        void* pEventData);

typedef void (*sipxMediaCallbackFn)(const void*      pSrc,
                                    const char*      szCallId,
                                    const char*      szRemoteAddress,
                                    SIPX_MEDIA_EVENT event,
                                    SIPX_MEDIA_CAUSE cause,
                                    SIPX_MEDIA_TYPE  type,
                                    void*            pEventData);

typedef void (*sipxLineEventCallbackFn)(const void* pSrc,
                                        const char* szLineIdentifier,
                                        SIPX_LINESTATE_EVENT event,
                                        SIPX_LINESTATE_CAUSE cause);	


typedef bool (*sipxEventCallbackFn)(const void* pSrc,
                    			    SIPX_EVENT_CATEGORY category, 
                                    void* pInfo);

                        

/* ============================ FUNCTIONS ================================= */

/**
 * Disable all listener callbacks -- events will be dropped
 */
void sipxDisableListeners() ;


/**
 * Enable all listener callbacks (default setting)
 */
void sipxEnableListeners() ;

/**
 * Destroy all calls and send simulated DESTROY events
 */
void sipxCallDestroyAll(const SIPX_INST hInst) ;

/** 
 * Remove/Destroy all Conferences
 */
void sipxConferenceDestroyAll(const SIPX_INST hInst) ;

/** 
 * Remove/Destroy all Lines
 */
void sipxLineRemoveAll(const SIPX_INST hInst) ;

/** 
 * Remove/Destroy all Publishers
 */
void sipxPublisherDestroyAll(const SIPX_INST hInst) ;

/** 
 * Remove/Destroy all subscriptions
 */
void sipxSubscribeDestroyAll(const SIPX_INST hInst) ;

/**
 * Fire events to interested listeners (call events only).
 */
void sipxFireCallEvent(const void* pSrc,
                       const char* szCallId,
                       SipSession* pSession,
                       const char* szRemoteAddress,
                       SIPX_CALLSTATE_EVENT event,
                       SIPX_CALLSTATE_CAUSE cause,
                       void* pEventData=NULL) ;

/**
 * Fires events to interested listener (media events only)
 */
void sipxFireMediaEvent(const void* pSrc,
                        const char* szCallId,
                        const char* szRemoteAddress,
                        SIPX_MEDIA_EVENT event,
                        SIPX_MEDIA_CAUSE cause,
                        SIPX_MEDIA_TYPE type,
                        void* pEventData = NULL) ;

/**
 * Fires a Line Event to the listeners.
 */
void sipxFireLineEvent(const void* pSrc,
                       const char* szLineIdentifier,
                       SIPX_LINESTATE_EVENT event,
                       SIPX_LINESTATE_CAUSE cause);

/**
 * Bubbles up all non-line and non-call events to the application layer
 */
bool sipxFireEvent(const void* pSrc,
                   SIPX_EVENT_CATEGORY category, 
                   void* pInfo);
                     
SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager) ;

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxCallReleaseLock(SIPX_CALL_DATA*, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxCallObjectFree(const SIPX_CALL hCall, const OsStackTraceLogger& oneBackInStack);
SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc);
void destroyCallData(SIPX_CALL_DATA* pData);
UtlBoolean validCallData(SIPX_CALL_DATA* pData);
UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pStrCallId,
                                 UtlString* pStrRemoteAddress,
                                 UtlString* pLineId,
                                 UtlString* pGhostCallId = NULL,
                                 UtlString* pContactAddress = NULL) ;

SIPX_CONF sipxCallGetConf(SIPX_CALL hCall) ;

SIPXTAPI_API UtlBoolean sipxCallGetState(SIPX_CALL hCall, 
                                         SIPX_CALLSTATE_EVENT& lastEvent,
                                         SIPX_CALLSTATE_CAUSE& lastCause,
                                         SIPX_INTERNAL_CALLSTATE& state) ;

UtlBoolean sipxCallGetMediaState(SIPX_CALL hCall,
                                 SIPX_MEDIA_EVENT& lastLocalMediaAudioEvent,
                                 SIPX_MEDIA_EVENT& lastLocalMediaVideoEvent,
                                 SIPX_MEDIA_EVENT& lastRemoteMediaAudioEvent,
                                 SIPX_MEDIA_EVENT& lastRemoteMediaVideoEvent) ;

UtlBoolean sipxCallSetMediaState(SIPX_CALL hCall,
                                 SIPX_MEDIA_EVENT event,
                                 SIPX_MEDIA_TYPE type) ;

UtlBoolean sipxCallSetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause) ;

SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) ;

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxLineObjectFree(const SIPX_LINE hLine) ;
SIPX_LINE sipxLineLookupHandle(const char* szLineURI, const char* requestUri); 
SIPX_LINE sipxLineLookupHandleByURI(const char* szURI); 
UtlBoolean validLineData(const SIPX_LINE_DATA*) ;

UtlBoolean sipxAddCallHandleToConf(const SIPX_CALL hCall,
                                   const SIPX_CONF hConf) ;

UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf,
                                        const SIPX_CALL hCall) ;

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxConfFree(const SIPX_CONF hConf) ;
UtlBoolean validConfData(const SIPX_CONF_DATA* pData) ;

void sipxIncSessionCount();
void sipxDecSessionCount();
int sipxGetSessionCount();

UtlBoolean sipxIsCallInFocus() ;

SIPXTAPI_API SIPX_RESULT sipxStructureIntegrityCheck();

const char* sipxContactTypeToString(SIPX_CONTACT_TYPE type) ;
const char* sipxTransportTypeToString(SIPX_TRANSPORT_TYPE type) ;

/**
 * Frees the INFO structure allocated by a call to sipxCallSendInfo
 *
 * @param pData Pointer to SIPX_INFO_DATA structure
 */
void sipxInfoFree(SIPX_INFO_DATA* pData);

/**
 * Releases the INFO handle created by a call to sipxCallSendInfo.
 * Also cals sipxInfoFree.
 *
 * @param hInfo Handle to the Info object
 */
void sipxInfoObjectFree(SIPX_INFO hInfo);

/**
 * Frees the TRANSPORT structure allocated by a call to sipxConfigExternalTransportAdd
 *
 * @param pData Pointer to SIPX_TRANSPORT_DATA structure
 */
void sipxTransportFree(SIPX_TRANSPORT_DATA* pData);

/**
 * Releases the TRANSPORT handle created sipxConfigExternalTransportAdd
 * Also cals sipxTransportFree.
 *
 * @param hInfo Handle to the Transport object
 */
void sipxTransportObjectFree(SIPX_TRANSPORT hTransport);

void sipxGetContactHostPort(SIPX_INSTANCE_DATA* pData, 
                            SIPX_CONTACT_TYPE   contactType, 
                            Url&                uri,
                            SIPX_TRANSPORT_TYPE protocol) ;
  //: Get the external host and port given the contact preference

/**
 * Looks up the SIPX_INFO_DATA structure pointer, given the SIPX_INFO handle.
 * @param hInfo Info Handle
 * @param type Lock type to use during lookup.
 */
SIPX_INFO_DATA* sipxInfoLookup(const SIPX_INFO hInfo, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);

/**
 * Unlocks the mutex associated with the INFO DATA
 * 
 * @param pData pointer to the SIPX_INFO structure
 * @param type Type of lock (read or write)
 */
void sipxInfoReleaseLock(SIPX_INFO_DATA* pData, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);

SIPX_PUBLISH_DATA* sipxPublishLookup(const SIPX_PUB hPub, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxPublishReleaseLock(SIPX_PUBLISH_DATA* pData, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);

SIPX_SUBSCRIPTION_DATA* sipxSubscribeLookup(const SIPX_SUB hSub, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);
void sipxSubscribeReleaseLock(SIPX_SUBSCRIPTION_DATA* pData, SIPX_LOCK_TYPE type, const OsStackTraceLogger& oneBackInStack);

/**
 * Looks up the SIPX_TRANSPORT_DATA structure pointer, given the SIPX_TRANSPORT handle.
 * @param hTransport Transport Handle
 * @param type Lock type to use during lookup.
 */
SIPX_TRANSPORT_DATA* sipxTransportLookup(const SIPX_TRANSPORT hTransport, SIPX_LOCK_TYPE type);

/**
 * Unlocks the mutex associated with the TRANSPORT DATA
 * 
 * @param pData pointer to the SIPX_TRANSPORT structure
 * @param type Type of lock (read or write)
 */
void sipxTransportReleaseLock(SIPX_TRANSPORT_DATA* pData, SIPX_LOCK_TYPE type);

/**
 * Adds a log entry to the system log - made necessary to add logging
 * capability on the API level.
 * 
 * @param logLevel priority of the log entry
 * @param format a format string for the following variable argument list
 */
SIPXTAPI_API void sipxLogEntryAdd(OsSysLogPriority logLevel, 
                     const char *format,
                     ...);

/**
 * Utility function for setting allowed methods on a 
 * instance's user-agent.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigAllowMethod(const SIPX_INST hInst, const char* method, const bool bAllow = true);

/**
 * Get the list of active calls for the specified call manager instance
 */
SIPXTAPI_API SIPX_RESULT sipxGetActiveCallIds(SIPX_INST hInst, int maxCalls, int& actualCalls, UtlString callIds[]) ;

/**
 * Callback for subscription client state
 */
void sipxSubscribeClientSubCallback(enum SipSubscribeClient::SubscriptionState newState,
                                   const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   int responseCode,
                                   const char* responseText,
                                   long expiration,
                                   const SipMessage* subscribeResponse);

/**
 * Callback for subscription client NOTIFY content
 */
void sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                     const char* dialogHandle,
                                     void* applicationData,
                                     const SipMessage* notifyRequest);

/**
 * Look for leaks in internal handles
 */
SIPXTAPI_API SIPX_RESULT sipxCheckForHandleLeaks() ;


/**
 * Flush handles to remove peaks between test cases -- this *WILL* leak 
 * memory.
 */
SIPXTAPI_API SIPX_RESULT sipxFlushHandles() ;


/**
 * Translate tone ids to implementation specific codes
 *
 * @param toneId sipx-internal tone id
 * @param xlateId implementation-specific tone id
 */
SIPXTAPI_API SIPX_RESULT sipxTranslateToneId(const TONE_ID toneId,
                                             TONE_ID& xlateId) ;


/**
 * Gets an CpMediaInterface pointer, associated with the call connection.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param ppInstData pointer to a memory address that is set to the media interface
 *        pointer.
 */                                                 
SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionMediaInterface(const SIPX_CALL hCall,
                                                           void** ppInstData);
                                                           
/**
 * Returns the 'local' connection id
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalAudioConnectionId(const SIPX_INST hInst, int& connectionId);
                                                           
                                                           
#ifdef VOICE_ENGINE
    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return the GipsVoiceEngineLib pointer associated with the
     * call.
     *
     * @param hCall Handle to a call.  Call handles are obtained either by 
     *        invoking sipxCallCreate or passed to your application through
     *        a listener interface.
     */
    SIPXTAPI_API GipsVoiceEngineLib* sipxCallGetVoiceEnginePtr(const SIPX_CALL hCall);


    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return the GipsVoiceEngineLib pointer associated with the
     * factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GipsVoiceEngineLib* sipxConfigGetVoiceEnginePtr(const SIPX_INST hInst);

#ifdef _WIN32
    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * return a Audio Tuning Wizard pointer associated with the
     * factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GIPSAECTuningWizard* sipxConfigGetVoiceEngineAudioWizard();
#endif

#ifdef VIDEO
    /**
     * For Gips VideoEngine versions of sipXtapi, this method will
     * return the GipsVideoEngine[Windows|Mac] pointer associated with
     * the factory implementation.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API GipsVideoEnginePlatform* sipxConfigGetVideoEnginePtr(const SIPX_INST hInst);
#endif VIDEO
    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * creates a Local Audio connection, which can be used to play
     * media files.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API SIPX_RESULT sipxCreateLocalAudioConnection(const SIPX_INST hInst);

    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * destroys the Local Audio connection, which was created by a
     * call to sipxCreateLocalAudioConnection.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     */
    SIPXTAPI_API SIPX_RESULT sipxDestroyLocalAudioConnection(const SIPX_INST hInst);

    /**
     * For Gips VoiceEngine versions of sipXtapi, this method will
     * enable or disable insertion of VoiceEngine trace output into the 
     * sipXtapi log.
     *
     * @param hInst Instance pointer obtained by sipxInitialize
     * @param bEnable Enable or disable VoceEngine trace output
     */
    SIPXTAPI_API SIPX_RESULT sipxEnableAudioLogging(const SIPX_INST hInst, bool bEnable);
#endif
                                                           

UtlBoolean sipxCallSetRemoveInsteadofDrop(SIPX_CALL hCall) ;
UtlBoolean sipxCallIsRemoveInsteadOfDropSet(SIPX_CALL hCall) ;

void sipxUpdateListeners(SIPX_INST hOldInst, SIPX_INST hNewInst) ;

/**
 * Dynamically loads the following NSS runtime libraries, if they can be found:
 * smime3.dll;nss3.dll;nspr4.dll;plc4.dll
 *
 * @returns SIPX_RESULT_SUCCESS if all NSS libraries were loaded.  Otherwise,
 *          SIPX_RESULT_MISSING_RUNTIME_FILES is returned.
 *        
 */
SIPXTAPI_API SIPX_RESULT sipxConfigLoadSecurityRuntime();

/**
 * Called from sipxConfigExternalTransportAdd, this function creates
 * LOCAL, STUN, and RELAY contact records for the newly added
 * transport mechanism.
 */
void sipxCreateExternalTransportContacts(const SIPX_TRANSPORT_DATA* pData);

class SecurityHelper
{
public:
    void generateSrtpKey(SIPX_SECURITY_ATTRIBUTES& securityAttrib);
    void setDbLocation(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbLocation);
    void setMyCertNickname(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* myCertNickname);
    void setDbPassword(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbPassword);
    
};



#endif /* ] _SIPXTAPIINTERNAL_H */
