// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#define OSBPROMPT_EXPORTS
#include "OSBprompt.h"
#include "osbprompt_tts.h"

#include <cstdio>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdlib.h>
#if defined(_WIN32)
    #include <windows.h>
    #define sleep Sleep
#elif defined(__pingtel_on_posix__)
    #include <unistd.h>
#endif
#define VXIstrcmp wcscmp
#include "VXIvalue.h"
#include "SBclientUtils.h"
#include "VXI/PromptManager.hpp"

#include "os/OsBSem.h"
#include "os/OsLock.h"
#include "mp/MpMediaTask.h"
#include "mp/NetInTask.h"
#include "mp/MpStreamQueuePlayer.h"
#include "rtcp/RTCManager.h"
#include "net/SipUserAgent.h"
#include "net/SdpCodecFactory.h"
/* #include "xmlparser/tinyxml.h" */
#ifdef DMALLOC
#include <dmalloc.h>
#endif

//uncomment next line to enable bounds checker in the build
//
//#define BOUNDS_CHECKER 1

#ifdef BOUNDS_CHECKER
        #include "C:\Program Files\Compuware\BoundsChecker\ERptApi\apilib.h"
        #pragma comment(lib, "C:\\Program Files\\Compuware\\BoundsChecker\\ERptApi\\nmapi.lib");
#endif


typedef std::basic_string<VXIchar> vxistring;

// ------*---------*---------*---------*---------*---------*---------*---------
#ifndef _WIN32
PlayerListenerDB** glbPlayerListenerTable = NULL;
int glbMaxNumListeners = 0;
int glbPlayerListenerCnt = 0;
#endif

OsMutex  glbPlayerListenerMutex(OsMutex::Q_FIFO);

// Global for the base diagnostic tag ID
//
static VXIunsigned gblDiagLogBase = 0;

// Global for the base prompt url
//
static const VXIchar *gblPromptUrlBase = 0;

// Constants for diagnostic logging tags
//
static const VXIunsigned DIAG_TAG_PROMPTING = 0;


#define OSBPROMPT_MAX_QUEUE_SIZE 100

// OSBprompt implementation of the VXIprompt interface
//
extern "C" {
    struct OSBpromptImpl
    {
        // Base interface, must be first
        VXIpromptInterface        intf;
        // Log interface for this resource
        VXIlogInterface           *log;
        CallManager               *pCallMgr;
        OsBSem                    *pExitGuard;
        VXIchar                   *callId;
        MpStreamQueuePlayer       *pPlayer;
        OSBQueuePlayerListener    *pPlayerListener;
        MpStreamQueuePlayer       *pTtsPlayer;
        UtlDList                   mTtsDlist;
        UtlDList                   mDlist;     // doubly-linked list used to store messages
        int                       live;       // session state 1 is alive, 0 exiting
        int                       hungup;       // 1 if caller has hung up and Exiting is called, set to 0 in AddResource.
        OsBSem                    *pWaitSem;  // demaphore to lock the blocking state for prompt play
        VXIunsigned               channel;
    };
}

static void VXIAddPlayerListenerToGlobalTable(const PlayerListenerDB* pListenerDB)
{
   OsLock lock(glbPlayerListenerMutex);
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                "VXIAddPlayerListenerToGlobalTable for channel %d callId %s listener %p\n", 
                pListenerDB->channelNum, pListenerDB->callId, pListenerDB->pListener);

        for (int i = 0; i < glbPlayerListenerCnt; i++)
        {
                if ((strcmp(glbPlayerListenerTable[i]->callId, pListenerDB->callId) == 0) &&
                         (glbPlayerListenerTable[i]->channelNum == pListenerDB->channelNum))
                {
                        glbPlayerListenerTable[i]->pListener->addRef();
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                                "VXIAddPlayerListenerToGlobalTable Listener already exists for call %s, channel %d\n", 
                                pListenerDB->callId, pListenerDB->channelNum);
                        return;
                }
        }

        if (glbPlayerListenerCnt == glbMaxNumListeners)
        {
                //make more of em.
                glbMaxNumListeners += 20;
                glbPlayerListenerTable = (PlayerListenerDB **)realloc(glbPlayerListenerTable,
                                                sizeof(PlayerListenerDB *)*glbMaxNumListeners);
                for (int loop = glbPlayerListenerCnt;loop < glbMaxNumListeners;loop++)
                        glbPlayerListenerTable[loop] = 0 ;
        }

        glbPlayerListenerTable[glbPlayerListenerCnt++] = (PlayerListenerDB*) pListenerDB;

}

OSBPROMPT_API OSBQueuePlayerListener* VXIGetPlayerListener(const char* callId, VXIunsigned channel)
{
   OsLock lock(glbPlayerListenerMutex);

        if (callId == NULL)
        {
                OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                        "VXIGetPlayerListener failed for channel %d callId is NULL!\n", channel);
                return NULL;
        }

   PlayerListenerDB* pMatchingListenerDB = NULL;
        OSBQueuePlayerListener* pMatchingListener = NULL;
   int found = -1;

        for (int i = 0; i < glbPlayerListenerCnt; i++)
        {
                if ((strcmp(glbPlayerListenerTable[i]->callId, callId) == 0) &&
                         (glbPlayerListenerTable[i]->channelNum == channel))
                {
                        found = i;
                        break;
                }
        }

        if (found >= 0)
        {
                pMatchingListenerDB = glbPlayerListenerTable[found];
                pMatchingListener = glbPlayerListenerTable[found]->pListener;

                int ref = pMatchingListener->addRef();
                OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                                "VXIGetPlayerListener found Listener %p for call %s, channel %d, mRef=%d\n", 
                                pMatchingListener, callId, channel, ref);
        }
        else
        {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                        "VXIGetPlayerListener failed for channel %d callId %s\n", 
                        channel, callId);
        }

   return(pMatchingListener);
}

OSBPROMPT_API int VXIReleasePlayerListener(const char* callId, VXIunsigned channel)
{
   OsLock lock(glbPlayerListenerMutex);
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                        "VXIReleasePlayerListener for channel %d callId %s\n", 
                        channel, callId); 

        OSBQueuePlayerListener* pMatchingListener = NULL;
   int found = -1;
   int i;
        for (i = 0; i < glbPlayerListenerCnt; i++)
        {
                if ((strcmp(glbPlayerListenerTable[i]->callId, callId) == 0) &&
                         (glbPlayerListenerTable[i]->channelNum == channel))
                {
                        found = i;
                        break;
                }
        }

   int ref = 1;
        if (found >= 0)
        {
                pMatchingListener = glbPlayerListenerTable[found]->pListener;
                if (pMatchingListener)
                {
                        ref = pMatchingListener->release();
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                                "VXIReleasePlayerListener found listener %p for channel %d callId %s, mRef=%d\n", 
                                pMatchingListener, channel, callId, ref);

                        if (ref <= 0)
                        {
                                delete pMatchingListener;
                                pMatchingListener = NULL;
                                delete glbPlayerListenerTable[found];

                                glbPlayerListenerCnt--;
                                for (i = found; i < glbPlayerListenerCnt; i++)
                                {
                                        glbPlayerListenerTable[i] = glbPlayerListenerTable[i + 1];
                                }
                                glbPlayerListenerTable[glbPlayerListenerCnt] = NULL;
                        }
                }
                else
                {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, 
                                "VXIReleasePlayerListener found listener %p for channel %d callId %s\n", 
                                pMatchingListener, channel, callId);
                }
        }
        else
        {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_INFO, 
                        "VXIReleasePlayerListener failed for channel %d callId %s, NOT FOUND.\n", 
                        channel, callId);
        }

   return ref;
}


// A few conversion functions...
static inline OSBpromptImpl * ToOSBpromptImpl(VXIpromptInterface * i)
{
    return reinterpret_cast<OSBpromptImpl *>(i);
}


/*******************************************************
 *
 * Utility functions
 *
 *******************************************************/

/**
 * Log an error
 */
/// static VXIlogResult Error(OSBpromptImpl *impl, VXIunsigned errorID,
///                           const VXIchar *format, ...)
/// {
///     VXIlogResult rc;
///     va_list arguments;
/// 
///     if ( (! impl) || (! impl->log) )
///         return VXIlog_RESULT_NON_FATAL_ERROR;
/// 
///     if ( format )
///     {
///         va_start(arguments, format);
///         rc = (*impl->log->VError)(impl->log, COMPANY_DOMAIN L".OSBprompt",
///                                   errorID, format, arguments);
///         va_end(arguments);
///     } else
///     {
///         rc = (*impl->log->Error)(impl->log, COMPANY_DOMAIN L".OSBprompt",
///                                  errorID, NULL);
///     }
///     return rc;
/// }


/**
 * Log a diagnostic message
 */
static VXIlogResult Diag(OSBpromptImpl *impl, VXIunsigned tag,
                         const VXIchar *subtag, const VXIchar *format, ...)
{
    VXIlogResult rc;
    va_list arguments;

    if ( (! impl) || (! impl->log) )
        return VXIlog_RESULT_NON_FATAL_ERROR;

    if ( format )
    {
        va_start(arguments, format);
        rc = (*impl->log->VDiagnostic)(impl->log, tag + gblDiagLogBase, subtag,
                                       format, arguments);
        va_end(arguments);
    } else
    {
        rc = (*impl->log->Diagnostic)(impl->log, tag + gblDiagLogBase, subtag, 
                                      NULL);
    }

    return rc;
}


/*******************************************************
 *
 * Method routines for VXIpromptInterface structure
 *
 *******************************************************/
// Get the VXIprompt interface version supported
//
static VXIint32 OSBpromptGetVersion(void)
{
    return VXI_CURRENT_VERSION;
}

// Get the implementation name
//
static const VXIchar* OSBpromptGetImplementationName(void)
{
    static const VXIchar IMPLEMENTATION_NAME[] = COMPANY_DOMAIN L".OSBprompt";
    return IMPLEMENTATION_NAME;
}

// Begin a session
//
static
VXIpromptResult OSBpromptBeginSession(VXIpromptInterface * pThis, VXIMap *sessionArgs)
{
#ifdef BOUNDS_CHECKER
        NMMemPopup( );
#endif

    VXIpromptResult res = VXIprompt_RESULT_SUCCESS;

    OSBpromptImpl *impl = ToOSBpromptImpl(pThis);
    impl->live = -1; // not quite live yet, other threads shouldn't its access resources

    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"prompt BeginSession");

    impl->pExitGuard = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
         
         if (impl->pExitGuard)
         {
                OsLock lock(*(impl->pExitGuard));

                const VXIchar *callId = NULL;
                const VXIValue *val;
                impl->pPlayer = NULL;
                impl->pTtsPlayer = NULL;
                impl->pWaitSem = NULL;
                impl->callId = NULL;
                impl->pPlayerListener = NULL;

                val = VXIMapGetProperty(sessionArgs, L"callid");
                if ( (val) && (VXIValueGetType(val) == VALUE_STRING) )
                {
                        callId = VXIStringCStr((const VXIString *) val);
                        int len = strlen((char*)callId) + 1;

                        VXIchar *vxiCallid = (VXIchar *) calloc(len, sizeof(VXIchar));
                        if ( vxiCallid )
                        {
            strncpy((char*)vxiCallid, (char*)callId, len);
            impl->callId = vxiCallid;
            MpStreamQueuePlayer* pPlayer = NULL;
            MpStreamQueuePlayer** ppPlayer = &pPlayer;

            impl->pCallMgr->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER,
                                         (const char*)callId,
                                         NULL,
                                         0,
                                         (MpStreamPlayer**)ppPlayer) ;
            if ( pPlayer )
            {
                impl->pPlayer = pPlayer;
                impl->pPlayerListener = new OSBQueuePlayerListener();
                if (!(impl->pPlayerListener))
                {
                   res = VXIprompt_RESULT_OUT_OF_MEMORY;
                }
                else
                {
                                                 PlayerListenerDB* pListenerDB = new PlayerListenerDB;
                                                 if (pListenerDB)
                                                 {
                                                        pListenerDB->callId             = (char*)(const char*)impl->callId;
                                                        pListenerDB->channelNum = impl->channel;
                                                        pListenerDB->pListener  = impl->pPlayerListener;
                                                        VXIAddPlayerListenerToGlobalTable(pListenerDB);
                                                                Diag(impl, DIAG_TAG_PROMPTING, NULL, L"VXIAddPlayerListenerToGlobalTable %p", impl->pPlayerListener);
                                                 }
                   if (impl->pPlayerListener) impl->pPlayer->addListener(impl->pPlayerListener);
                }
            }
            else
            {
               res = VXIprompt_RESULT_INVALID_ARGUMENT;
            }

                        } else
                                        res = VXIprompt_RESULT_OUT_OF_MEMORY;
                }
                else
                        res = VXIprompt_RESULT_INVALID_ARGUMENT;

                if (impl->hungup != 1 && res == VXIprompt_RESULT_SUCCESS)
                {
                        impl->pWaitSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
                        impl->live = 1; // live
                }
                else
                {
                        impl->live = 0; // exiting
                        Diag(impl, DIAG_TAG_PROMPTING, NULL, 
                                                        L"prompt BeginSession failed, setting live to 0");

                        // free all resources because we had an error
                        if ( impl->pPlayerListener )
                        {
                                if(impl->pPlayer) 
                                {
                                        impl->pPlayer->removeListener(impl->pPlayerListener);
                                }

                                if (impl->pPlayer)
                                {       
                                        impl->pCallMgr->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER,
                                         (const char*)impl->callId,
                                         (MpStreamPlayer *)impl->pPlayer);
                                        impl->pPlayer = NULL;
                                }

                                int ref = VXIReleasePlayerListener((const char*)impl->callId, (unsigned int)impl->channel);
                                Diag(impl, DIAG_TAG_PROMPTING, NULL, 
                                                        L"releasePlayerListener %p returned %d",
                                                        impl->pPlayerListener, ref);
                                impl->pPlayerListener = NULL;
                        }

                        if (impl->callId)
                        {
                                free(impl->callId);
                                impl->callId = NULL;
                        }

                }
         }
         else
         {
                        impl->live = 0; // exiting
         Diag(impl, DIAG_TAG_PROMPTING, NULL,
                  L"prompt BeginSession failed, could not create semaphore.");

         }

    return res;
}


// End a session
//
static
VXIpromptResult OSBpromptEndSession(VXIpromptInterface *pThis, VXIMap *sessionArgs)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(pThis);
    
    if (!impl) return VXIprompt_RESULT_INVALID_ARGUMENT;

    if (impl->pExitGuard)
    {
        impl->pExitGuard->acquire();
        impl->live = 0;
                Diag(impl, DIAG_TAG_PROMPTING, NULL, L"End prompt session");
                if ( impl->pPlayer )
                {
                        MpStreamQueuePlayer *pPlayer = impl->pPlayer;
                impl->pPlayer = NULL; // set to null so not to be used any more.

                        pPlayer->reset() ;

                        if ( impl->pPlayerListener )
                        {
                                        pPlayer->removeListener(impl->pPlayerListener);

                                        impl->pCallMgr->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER,
                                      (const char*)impl->callId,
                                      (MpStreamPlayer *)pPlayer);

                                        int ref = VXIReleasePlayerListener((const char*)impl->callId, (unsigned int)impl->channel);
                                        Diag(impl, DIAG_TAG_PROMPTING, NULL, 
                                                        L"prompt EndSession, VXIReleasePlayerListener %p, ref=%d",
                                                        impl->pPlayerListener, ref);
                                        impl->pPlayerListener = NULL;
                        }

                }
                free(impl->callId);
                impl->callId = NULL;
                if ( impl->pWaitSem ) impl->pWaitSem->acquire();
                while ( impl->mDlist.entries() > 0 )
                {
                        UtlInt* audio = (UtlInt*) impl->mDlist.at(0);
                        // remove the message from the queue
                        UtlInt* pRemoved = (UtlInt*) impl->mDlist.removeReference(audio);
            if (pRemoved != NULL)
            {
                delete pRemoved ;
            }
                }

                if ( impl->pWaitSem )
                {
                        impl->pWaitSem->release();
                        delete impl->pWaitSem;
                        impl->pWaitSem = 0;
                }
        impl->pExitGuard->release();
    }
#ifdef BOUNDS_CHECKER
        NMMemPopup( );
#endif
    return VXIprompt_RESULT_SUCCESS;
}


// Stop playing queued segments, non-blocking
VXIpromptResult OSBpromptStop(struct VXIpromptInterface  *pThis)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(pThis);
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Stop playing queued prompts");
    if ( (impl->live == 1) && impl->pPlayer )
    {
        impl->pPlayer->reset() ;
    }
    return VXIprompt_RESULT_SUCCESS;
}

// add listener to player
VXIpromptResult OSBpromptAddPlayerListener(struct VXIpromptInterface  *pThis,
                                           void* pListener)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(pThis);
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Add player listener");
    if ( (impl->live == 1) && impl->pPlayer )
    {
        impl->pPlayer->addListener((OSBQueuePlayerListener*)pListener);
    }
    return VXIprompt_RESULT_SUCCESS;
}

// Start playing queued prompts.
//
static VXIpromptResult OSBpromptPlay(VXIpromptInterface * vxip)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(vxip);
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Playing queued prompts");

    if ( impl->pWaitSem ) impl->pWaitSem->acquire();
    int cnt = impl->mDlist.entries();
    if ( impl->callId && (cnt > 0) )
    {
        if (impl->live == 1)
        {
            impl->pPlayer->play() ;
        }
        UtlInt* audio;
        int bargein;
        int tts;
        int blocking = 0;
        for ( int i = 0; i < cnt; i++ )
        {
            audio = (UtlInt*) impl->mDlist.at(0);  // peek at the first message
            impl->mDlist.removeReference(audio);
            bargein = audio->getValue() ;
            delete audio ;
            tts = (bargein >> 2);
            blocking += ((bargein != 0) && ((bargein & 0x0003) != PromptManager::ENABLED));
        }
        Diag(impl, DIAG_TAG_PROMPTING, NULL, L"bargein = %d", !blocking);
        if ( (impl->live == 1) && blocking ) impl->pPlayer->wait() ;
    }
    if ( impl->pWaitSem ) impl->pWaitSem->release();
    return VXIprompt_RESULT_SUCCESS;
}


// Start the special play of a filler prompt. This call is non-blocking.
//
static
VXIpromptResult OSBpromptPlayFiller(VXIpromptInterface * vxip,
                                    const VXIchar *type,
                                    const VXIchar *src,
                                    const VXIchar *text,
                                    const VXIMap* properties,
                                    VXIlong minPlayMsec)
{
    return VXIprompt_RESULT_SUCCESS;
}


// Start the special play of a filler prompt. This call is non-blocking.
//
static
VXIpromptResult OSBpromptPrefetch(VXIpromptInterface * vxip,
                                  const VXIchar *type,
                                  const VXIchar *src,
                                  const VXIchar *text,
                                  const VXIMap* properties)
{
    return VXIprompt_RESULT_SUCCESS;
}


// Queue prompt for playing. This call is non-blocking. By default, a
//  prompt added to an empty queue starts playing immediately.
//
static
VXIpromptResult OSBpromptQueue(VXIpromptInterface* vxip,
                               const VXIchar *raw_type,
                               const VXIchar *raw_src,
                               const VXIchar *raw_text,
                               const VXIMap  *properties,
                               int           bargein)
{
#if 0
    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
		  "OSBpromptQueue(vxip = %p, raw_type = '%ls', raw_src = '%ls', raw_text = '%ls', properties = %p, bargein = %d",
		  vxip, (raw_type ? raw_type : L"(null)"),
		  (raw_src ? raw_src : L"(null)"),
		  (raw_text ? raw_text : L"(null)"), properties, bargein);
#endif
    OSBpromptImpl *impl = ToOSBpromptImpl(vxip);

    const VXIbyte *binaryData = NULL;
    VXIulong binarySizeBytes = 0;
    vxistring type, src, text;
    const VXIchar* baseUrl = 0;
    if ( raw_type )
        type = raw_type;
    if ( raw_src )
        src = raw_src;
    if ( raw_text )
        text = raw_text;

    // Resolve audio references

    const VXIValue * urltemp = VXIMapGetProperty(properties, INET_URL_BASE);
    if ( urltemp ) baseUrl = VXIStringCStr(reinterpret_cast<const VXIString *>(urltemp));

    if ( src.find(PROMPT_AUDIO_REFS_SCHEME) == 0 )
    {
        src.erase(0, PROMPT_AUDIO_REFS_SCHEME_LENGTH);

        // Get audio references from properties.
        const VXIValue * temp = VXIMapGetProperty(properties, PROMPT_AUDIO_REFS);
        if ( temp == NULL || VXIValueGetType(temp) != VALUE_MAP )
            return VXIprompt_RESULT_INVALID_ARGUMENT;


        // Find information for this prompt.
        temp = VXIMapGetProperty((const VXIMap *) temp, src.c_str());
        if ( temp == NULL || VXIValueGetType(temp) != VALUE_MAP )
            return VXIprompt_RESULT_INVALID_ARGUMENT;

        const VXIMap * ref = (const VXIMap *) temp;
        type = L"";
        src = L"";
        text = L"";

        // Get the type
        temp = VXIMapGetProperty(ref, PROMPT_AUDIO_REF_TYPE);
        if ( temp != NULL )
        {
            if ( VXIValueGetType(temp) == VALUE_STRING )
                type = VXIStringCStr((const VXIString *) temp);
            else
                return VXIprompt_RESULT_INVALID_ARGUMENT;
        }

        // Get the URL
        temp = VXIMapGetProperty(ref, PROMPT_AUDIO_REF_SRC);
        if ( temp != NULL )
        {
            if ( VXIValueGetType(temp) != VALUE_STRING )
                return VXIprompt_RESULT_INVALID_ARGUMENT;
            else
                src = VXIStringCStr((const VXIString *) temp);
        }

        // Get the text or binary data
        temp = VXIMapGetProperty(ref, PROMPT_AUDIO_REF_DATA);
        if ( temp != NULL )
        {
            switch ( VXIValueGetType(temp) )
            {
            case VALUE_CONTENT: {
                    const VXIchar *contentType;
                    if ( VXIContentValue((const VXIContent *) temp, &contentType,
                                         &binaryData, &binarySizeBytes) ==
                         VXIvalue_RESULT_SUCCESS )
                        type = contentType;
                    else
                        return VXIprompt_RESULT_INVALID_ARGUMENT;
                } break;

            case VALUE_STRING:
                text = VXIStringCStr((const VXIString *) temp);
                break;

            default:
                return VXIprompt_RESULT_INVALID_ARGUMENT;
            }
        }
    }

    // Handle the resolved information to queue.

    if ( ! src.empty() )
    {
#if 0
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
		      "OSBpromptQueue: handling as src = '%ls'", src.c_str());
#endif
        unsigned int len = src.length();
        Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Queuing AUDIO: %s", src.c_str());

        char* str = new char[len + 1];
        //  wcstombs((char*)str, src.c_str(), len);
        unsigned int i;
        //  memcpy((char*)str, (const char*)src.c_str(), len);
        if ( str )
        {
            for ( i = 0; i < len; i++ )
               str[i] = src[i];
            str[len] = 0;
#if 0
	    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
			  "OSBpromptQueue: str");
	    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
			  "OSBpromptQueue: str = %p", str);
	    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
			  "OSBpromptQueue: str = '%s'", str);
#endif
            Url audiourl;
            // @JC Fixed bug here that interpreted HTTPS url's as relative
            if ( strstr(str, "https:") || strstr(str, "http:") || strstr(str, "file:") )
            {
                audiourl = Url(str);
            } else
            {
                // relative url
                UtlString basestr;
                if ( baseUrl && (len = wcslen(baseUrl)) > 0 )
                {
                    char *base = new char[len + 1];
                    for ( i = 0; i < len; i++ )
                        base[i] = baseUrl[i];
                    base[len] = 0;
                    basestr = UtlString(base);
                    if ( (len = basestr.last('/')) != UTL_NOT_FOUND )
                        basestr = basestr(0, (len + 1));
                    basestr.append(str);
                    audiourl = Url(basestr);
                    if ( base )
                    {
                        delete[] base;
                        base = NULL;
                         }
                } else
                {
                    audiourl = Url(str);
                }
            }
#if 0
	    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
			  "OSBpromptQueue: audiourl = '%s'",
			  audiourl.toString().data());
#endif

            if ( impl->pWaitSem )
                impl->pWaitSem->acquire();

            // Add the url to the playlist
            if ( impl->pPlayer )
            {
                impl->pPlayer->add(
                    audiourl,
                    STREAM_SOUND_REMOTE |
                    STREAM_FORMAT_WAV |
                    STREAM_HINT_CACHE);
                impl->mDlist.insert(new UtlInt(bargein));
            }

            if ( impl->pWaitSem )
                impl->pWaitSem->release();

            delete[] str;
            str = NULL;
            return VXIprompt_RESULT_SUCCESS;
        } else
        {
            return VXIprompt_RESULT_OUT_OF_MEMORY;
        }
    } else if ( !text.empty() )
    {
#if 0
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
		      "OSBpromptQueue: handling as text");
        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
		      "OSBpromptQueue: handling as text = '%ls'",
		      text.c_str());
#endif
        if ( type == VXI_MIME_XML || type == VXI_MIME_SSML )
        {
            // The language is encoded in the TTS string.
#if 0
	    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
			  "OSBpromptQueue: Queuing TTS, type = '%ls', text = '%ls'",
			  type.c_str(), text.c_str());
#endif
            Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Queuing TTS (%s): %s",
                 type.c_str(), text.c_str());
            OSBpromptTTS tts(raw_text, baseUrl, gblPromptUrlBase);
            Url* prompts = 0;
            int count = tts.getPromptUrls(&prompts);

            if ( impl->pWaitSem )
                impl->pWaitSem->acquire();

            // Add the array of prompts to the player's playlist
            for ( int i = 0; i < count; i++ )
            {
                if (impl->pPlayer)
                         impl->pPlayer->add(
                                                        prompts[i],
                                                        STREAM_SOUND_REMOTE |
                                                        STREAM_FORMAT_WAV |
                                                        STREAM_HINT_CACHE);
            }

            bargein = (bargein | (1<<2));    // extra bit marking TTS prompts.
            if ( count > 0 )
                impl->mDlist.insert(new UtlInt(bargein));

            if ( impl->pWaitSem )
                impl->pWaitSem->release();

        } else if ( type == VXI_MIME_TEXT || type == VXI_MIME_UNICODE_TEXT )
        {
            // Get the language.
            const VXIValue * lang = VXIMapGetProperty(properties, PROMPT_LANGUAGE);
            if ( lang != NULL && VXIValueGetType(lang) != VALUE_STRING )
                return VXIprompt_RESULT_INVALID_ARGUMENT;

            Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Queuing TTS in %s (%s): %s",
                 (lang == NULL) ? L"" : VXIStringCStr((const VXIString *) lang),
                 type.c_str(), text.c_str());
        } else
        {
            Diag(impl, DIAG_TAG_PROMPTING, NULL,
                 L"Queuing Unknown type text (%s): %s" , type.c_str(), text.c_str());
            return VXIprompt_RESULT_UNSUPPORTED;
        }
    } else if ( binarySizeBytes > 0 )
    {
        if ( type.find(L"audio/") == 0 )
        {
            Diag(impl, DIAG_TAG_PROMPTING, NULL,
                 L"Queuing IN-MEMORY AUDIO: (%s, %lu bytes)",
                 type.c_str(), binarySizeBytes);

            UtlString *audioBuf =
                new UtlString((const char*)binaryData, binarySizeBytes) ;
            if ((impl->live == 1) && audioBuf)
            {
                if ( impl->pWaitSem )
                    impl->pWaitSem->acquire();
                // Add an audio buffer to the a playlist
                impl->pPlayer->add(
                    audioBuf,
                    STREAM_SOUND_REMOTE |
                    STREAM_FORMAT_WAV |
                    STREAM_HINT_CACHE );

                impl->mDlist.insert(new UtlInt(bargein));
                delete audioBuf;
                audioBuf = NULL;

                if ( impl->pWaitSem )
                    impl->pWaitSem->release();
            }

        } else
        {
            Diag(impl, DIAG_TAG_PROMPTING, NULL,
                 L"Queueing Unknown type binary data (%s, %lu bytes)",
                 type.c_str(), binarySizeBytes);
            return VXIprompt_RESULT_UNSUPPORTED;
        }
    } else
    {
        return VXIprompt_RESULT_INVALID_ARGUMENT;
    }
    return VXIprompt_RESULT_SUCCESS;
}


// Wait until all queued prompts finish playing.
static
VXIpromptResult OSBpromptWait(VXIpromptInterface* vxip,
                              VXIpromptResult* playResult)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(vxip);
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"wait");
    return VXIprompt_RESULT_SUCCESS;
}

/*******************************************************
 * Global init and factory methods
 *******************************************************/

OSBPROMPT_API VXIpromptResult OSBpromptInit (VXIlogInterface  *log,
                                             VXIunsigned    diagLogBase,
                                             const VXIchar  *baseUrl)
{
    if ( ! log ) return VXIprompt_RESULT_INVALID_ARGUMENT;

    gblDiagLogBase = diagLogBase;
    gblPromptUrlBase = baseUrl;
    return VXIprompt_RESULT_SUCCESS;
}


OSBPROMPT_API VXIpromptResult OSBpromptShutDown (VXIlogInterface  *log)
{
    if ( ! log ) return VXIprompt_RESULT_INVALID_ARGUMENT;

    return VXIprompt_RESULT_SUCCESS;
}


OSBPROMPT_API VXIpromptResult OSBpromptExiting (VXIlogInterface  *log,
                                                VXIpromptInterface **prompt)
{
    if ( ! log ) return VXIprompt_RESULT_INVALID_ARGUMENT;

    OSBpromptImpl *impl = ToOSBpromptImpl(*prompt);
    if ( !impl ) return VXIprompt_RESULT_INVALID_ARGUMENT;

    impl->hungup = 1;  
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Shutting down Prompt");

    if ((impl->live == 1) && impl->pExitGuard)
    {
        impl->pExitGuard->acquire();
                impl->live = 0;
        impl->pExitGuard->release();
    }

    return VXIprompt_RESULT_SUCCESS;
}


OSBPROMPT_API VXIpromptResult OSBpromptCreateResource (
    VXIunsigned channelNum,
    VXIlogInterface     *log,
    VXIinetInterface    *inet,
    VXIpromptInterface **prompt,
    CallManager *pCallMgr)
{
    if ( (! log) || (! inet) ) return VXIprompt_RESULT_INVALID_ARGUMENT;

    OSBpromptImpl* pp = new OSBpromptImpl();
    if ( pp == NULL ) return VXIprompt_RESULT_OUT_OF_MEMORY;

    pp->log = log;
    pp->channel = channelNum;
    pp->pCallMgr = pCallMgr;
    pp->intf.GetVersion            = OSBpromptGetVersion;
    pp->intf.GetImplementationName = OSBpromptGetImplementationName;
    pp->intf.BeginSession          = OSBpromptBeginSession;
    pp->intf.EndSession            = OSBpromptEndSession;
    pp->intf.Play                  = OSBpromptPlay;
    pp->intf.Stop                  = OSBpromptStop;
    pp->intf.PlayFiller            = OSBpromptPlayFiller;
    pp->intf.Prefetch              = OSBpromptPrefetch;
    pp->intf.Queue                 = OSBpromptQueue;
    pp->intf.Wait                  = OSBpromptWait;
    pp->intf.AddPlayerListener     = OSBpromptAddPlayerListener;

    *prompt = &pp->intf;
    return VXIprompt_RESULT_SUCCESS;
}


OSBPROMPT_API
VXIpromptResult OSBpromptAddResource (VXIpromptInterface **prompt,
                                      CallManager *pCallMgr)
{
    OSBpromptImpl *impl = ToOSBpromptImpl(*prompt);
    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"Adding call manager to Prompt");

    impl->pCallMgr = pCallMgr;
    impl->live = -1;
    impl->hungup = 0;

    return VXIprompt_RESULT_SUCCESS;
}


OSBPROMPT_API
VXIpromptResult OSBpromptDestroyResource (VXIpromptInterface **prompt)
{
    if ( prompt == NULL || *prompt == NULL )
        return VXIprompt_RESULT_INVALID_ARGUMENT;

    OSBpromptImpl* impl = reinterpret_cast<OSBpromptImpl*>(*prompt);
     
    if (!impl) return VXIprompt_RESULT_INVALID_ARGUMENT;

    Diag(impl, DIAG_TAG_PROMPTING, NULL, L"prompt DestroyResource");
    if (impl->pExitGuard)
    {
                impl->pExitGuard->acquire();
                if (impl->live == 1)
                {
                   Diag(impl, DIAG_TAG_PROMPTING, NULL, L"live=1, call EndSession now. ");
                   impl->pExitGuard->release();
                        /* UNUSED VARIABLE VXIpromptResult res = */ OSBpromptEndSession(*prompt, NULL);
                   impl->pExitGuard->acquire();
                }
      impl->live = 0;
                impl->pExitGuard->release();
                delete impl->pExitGuard;
                impl->pExitGuard = NULL;
    }
    delete impl;
    impl = NULL;
    *prompt = NULL;

    return VXIprompt_RESULT_SUCCESS;
}
