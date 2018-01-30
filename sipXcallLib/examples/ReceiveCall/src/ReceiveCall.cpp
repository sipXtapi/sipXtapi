//  
// Copyright (C) 2006-2017 SIPez LLC. All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#  include "ReceiveCallWntApp.h"
DWORD WINAPI ConsoleStart(LPVOID lpParameter);
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "ExternalTransport.h"

// sipXtapi apps should not be digging into the lower layers
//#include <mi/CpMediaInterfaceFactoryImpl.h>

#define SAMPLES_PER_FRAME   80          // Number of samples per frame time
#define LOOPBACK_LENGTH     200         // Frames for loopback delay (10ms per frame)
#define MAX_CONFERENCE_LEGS 10          // Maximum number of calls per conference

#define portIsValid(p) ((p) >= 1 && (p) <= 65535)

static bool gbShutdown = false;         // Have shutdown been requested by user?
static bool gbOneCallMode = false;      // Should we exit after first call?
SIPX_INST g_hInst = NULL ;              // Handle to the sipXtapi instance
static short* g_loopback_samples[LOOPBACK_LENGTH] ; // loopback buffer
static short g_loopback_head = 0 ;      // index into loopback
static char* g_szPlayTones = NULL ;     // tones to play on answer
static SIPX_CALL g_playTonesNow = SIPX_CALL_NULL;
static char* g_szFile = NULL ;          // file to play on answer
static char* g_szRecordFile = NULL;     // Filename to record to
static bool gbConf = false;             // Act as conference bridge for all incoming calls
SIPX_CONF g_conf = SIPX_CONF_NULL;      // Single conference instance
static bool g_recordingFile = false;

#if defined(_WIN32) && defined(VIDEO)
extern HWND ghPreview;
extern HWND ghVideo;
extern HWND hMain;
static SIPX_VIDEO_DISPLAY gDisplay;
static SIPX_VIDEO_DISPLAY gPreviewDisplay;
static bool  bVideo = false;
#endif

bool bUseCustomTransportReliable = false;
bool bUseCustomTransportUnreliable = false;
SIPX_TRANSPORT ghTransport = SIPX_TRANSPORT_NULL;
void startTribbleListener(const char* szIp);
bool tribbleProc(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData) ;                            
void startFlibbleListener(const char* szIp);
bool flibbleProc(SIPX_TRANSPORT hTransport,
                 const char* szDestinationIp,
                 const int   iDestPort,
                 const char* szLocalIp,
                 const int   iLocalPort,
                 const void* pData,
                 const size_t nData,
                 const void* pUserData) ;
FlibbleTask* gpFlibbleTask = NULL;
SIPX_CONTACT_ID gContactId = CONTACT_AUTO;
SIPX_CONTACT_ADDRESS*   gpExternalTransportContactRecord;

static void ctrlCHandler(int signo)
{
   printf("\nShutting down...\n");
   gbShutdown = true;
}


// Print usage message
void usage(const char* szExecutable)
{
    char szBuffer[64] = "";

    sipxConfigGetVersion(szBuffer, 64);
    printf("\nUsage:\n");
    printf("   %s <options>\n", szExecutable) ;
    printf("      using %s\n", szBuffer) ;
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -d durationInSeconds (default=30 seconds)\n") ;
    printf("   -t playT tones (default = none)\n") ;
    printf("   -f play file (default = none)\n") ;
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -r RTP port start (default = 9000)\n") ;
    printf("   -B ip address to bind to\n");
    printf("   -conf mix all calls to a conference\n");
    printf("   -l loopback audio (2 second delay)\n") ;
    printf("   -1 one call mode (exit after first call end)\n") ;
    printf("   -I <string> call input device name\n");
    printf("   -O <string> call output device name\n");
    printf("   -drate <int> device sample rate (e.g. 16000, 32000, 48000)\n");
    printf("          Generally should run device sampling at the hardware native rate\n");
    printf("   -mrate <int> media/flowgraph sample rate (e.g. 8000, 16000, 32000, 48000)\n");
    printf("   -i line identity (e.g. sip:122@pingtel.com)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -rec Record to wave file (PCM)\n");
    printf("   -ref registration refresh period (default=3600 seconds)\n");
    printf("   -x proxy (outbound proxy)\n");
    printf("   -S stun server\n") ;
    printf("   -v show sipXtapi version\n");
#ifdef VIDEO
    printf("   -V receive video calls.\n");
#endif
    printf("   -E use bogus custom external transport, reliable (transport=tribble)\n");
    printf("   -e use bogus custom external transport, unreliable (transport=flibble)\n");

    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char *argv[],
               int* pDuration,
               int* pSipPort,
               int* pRtpPort,
               char** pszBindAddress,
               char** pszInputDevice,
               char** pszOutputDevice,
               char** pszPlayTones,
               char** pszFile,
               bool* bLoopback,
               bool* bOneCallMode,
               char** pszIdentity,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               int* pRefreshPeriod,
               int* pDeviceRate,
               int* pMediaRate,
               char** pszStunServer,
               char** pszProxy,
               char** pRecordFile)
{
    bool bRC = true ;
    char szBuffer[64];

    assert(pDuration && pszPlayTones) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pszBindAddress = NULL;
    *pszInputDevice = NULL;
    *pszOutputDevice = NULL;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *bLoopback = false ;
    *bOneCallMode = false ;
    *pszIdentity = NULL ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pRefreshPeriod = 0;
    *pDeviceRate = 48000;
    *pMediaRate = 8000;
    *pszStunServer = NULL ;
    *pszProxy = NULL;
    *pRecordFile = NULL;

    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            if ((i+1) < argc)
            {
                *pDuration = atoi(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            if ((i+1) < argc)
            {
                *pszPlayTones = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if ((i+1) < argc)
            {
                *pszFile = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            if ((i+1) < argc)
            {
                *pSipPort = atoi(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            if ((i+1) < argc)
            {
                *pRtpPort = atoi(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-B") == 0)
        {
            if ((i+1) < argc)
            {
                *pszBindAddress = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            *bLoopback = true ;
        }
        else if (strcmp(argv[i], "-1") == 0)
        {
           *bOneCallMode = true ;
        }
        else if (strcmp(argv[i], "-conf") == 0)
        {
           gbConf = true;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if ((i+1) < argc)
            {
                *pszIdentity = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }

        else if (strcmp(argv[i], "-u") == 0)
        {
            if ((i+1) < argc)
            {
                *pszUsername = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }

        else if (strcmp(argv[i], "-a") == 0)
        {
            if ((i+1) < argc)
            {
                *pszPassword = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }

        else if (strcmp(argv[i], "-m") == 0)
        {
            if ((i+1) < argc)
            {
                *pszRealm = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-rec") == 0)
        {
            if ((i + 1) < argc)
            {
                *pRecordFile = strdup(argv[++i]);
            }
            else
            {
                bRC = false;
                break; // Error
            }
        }

        else if (strcmp(argv[i], "-ref") == 0)
        {
            if ((i + 1) < argc)
            {
                *pRefreshPeriod = atoi(argv[++i]);
            }
            else
            {
                bRC = false;
                break; // Error
            }
        }

        else if (strcmp(argv[i], "-S") == 0)
        {
            if ((i+1) < argc)
            {
                *pszStunServer = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            if ((i+1) < argc)
            {
                *pszProxy = strdup(argv[++i]) ;
            }
            else
            {
                bRC = false ;
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            sipxConfigGetVersion(szBuffer, 64);
            printf("%s\n", szBuffer);
            exit(0);
        }
        else if (strcmp(argv[i], "-I") == 0)
        {
            if ((i+1) < argc)
            {
                *pszInputDevice = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-O") == 0)
        {
            if ((i+1) < argc)
            {
                *pszOutputDevice = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }

        else if (strcmp(argv[i], "-drate") == 0)
        {
            if ((i + 1) < argc)
            {
                *pDeviceRate = atoi(argv[++i]);
            }
            else
            {
                bRC = false;
                break; // Error
            }
        }

        else if (strcmp(argv[i], "-mrate") == 0)
        {
            if ((i + 1) < argc)
            {
                *pMediaRate = atoi(argv[++i]);
            }
            else
            {
                bRC = false;
                break; // Error
            }
        }

        else if (strcmp(argv[i], "-V") == 0)
        {
#ifdef VIDEO
            bVideo = true;
#else
            printf("Build with VIDEO not defined/enabled\n");
            break;  // Error
#endif
        }
        else if (strcmp(argv[i], "-E") == 0)
        {
            bUseCustomTransportReliable = true;            
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            bUseCustomTransportUnreliable = true;            
        }
        else
        {
            bRC = false ;
            break ; // Error
        }
    }

    return bRC ;
}

// Play a file (8000 samples/sec, 16 bit unsigned, mono PCM)
bool playFile(char* szFile, SIPX_CALL hCall)
{
    bool repeat = true;
    sipxCallAudioPlayFileStart(hCall, g_szFile, repeat, true, true) ;

    return true ;
}

// Play a series of tones
bool playTones(char* szPlayTones, SIPX_CALL hCall)
{
    bool bRC = true ;

    while (*szPlayTones)
    {
        int toneId = *szPlayTones++ ;

        if (    (toneId >= '0' && toneId <= '9') ||
            (toneId == '#') || (toneId == '*') || toneId == ',' || toneId == '!')
        {
            if (toneId == ',')
            {
                printf("<-> Playtone: Sleeping for 2 seconds\n") ;
                SLEEP(2000) ;
            }
            else
            {
                printf("<-> Playtone: %c\n", toneId) ;
                SLEEP(250) ;
                sipxCallStartTone(hCall, (SIPX_TONE_ID) toneId, true, false) ;
                SLEEP(500) ;
                sipxCallStopTone(hCall) ;
            }
        }
        else
        {
            bRC = false ;
            break ;
        }
    }

    return bRC ;
}

SIPX_RESULT recordFile(char* szFile, SIPX_CALL hCall)
{
    return(sipxCallAudioRecordFileStart(hCall, 
                                        szFile, 
                                        SIPX_WAVE_PCM_16, // format
                                        false, // append to recording file
                                        4)); // channels
}

void SpkrAudioHook(const int nSamples, short* pSamples)
{
    memcpy(g_loopback_samples[g_loopback_head], pSamples, sizeof(short) * SAMPLES_PER_FRAME) ;
    g_loopback_head = ((g_loopback_head + 1) % LOOPBACK_LENGTH) ;
    memset(pSamples, 0, sizeof(short) * SAMPLES_PER_FRAME) ;
}


void MicAudioHook(const int nSamples, short* pSamples)
{
    short index = ((g_loopback_head + 1) % LOOPBACK_LENGTH) ;
    memcpy(pSamples, g_loopback_samples[index], sizeof(short) * SAMPLES_PER_FRAME) ;
}


void clearLoopback()
{
    for (int i=0; i<LOOPBACK_LENGTH; i++)
    {
        if (g_loopback_samples[i])
        {
            memset(g_loopback_samples[i], 0, sizeof(short) * SAMPLES_PER_FRAME) ;
        }
    }
    g_loopback_head = 0 ;
}


void initLoopback()
{
    for (int i=0; i<LOOPBACK_LENGTH; i++)
    {
        g_loopback_samples[i] = new short[SAMPLES_PER_FRAME] ;
    }
    clearLoopback() ;

    sipxConfigSetSpkrAudioHook(SpkrAudioHook) ;
    sipxConfigSetMicAudioHook(MicAudioHook) ;
}


bool EventCallBack(SIPX_EVENT_CATEGORY category, 
                   void* pInfo, 
                   void* pUserData)
{
    assert (pInfo != NULL);

    // Dump event
    char cBuf[1024] ;
    printf("%s\n", sipxEventToString(category, pInfo, cBuf, sizeof(cBuf))) ;    

    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = static_cast<SIPX_CALLSTATE_INFO*>(pInfo);
        printf("    hCall=%d, hAssociatedCall=%d\n", pCallInfo->hCall, pCallInfo->hAssociatedCall) ;

        switch (pCallInfo->event)
        {
        case CALLSTATE_OFFERING:
#if defined(_WIN32) && defined(VIDEO)
            gDisplay.type = SIPX_WINDOW_HANDLE_TYPE;
            gDisplay.handle = ghVideo;
            if (bVideo)
            {
                sipxCallAccept(pCallInfo->hCall, &gDisplay) ;
            }
            else
#endif
 
            {
                sipxCallAccept(pCallInfo->hCall);
            }
            break;

        case CALLSTATE_ALERTING:
            clearLoopback();
            sipxCallAnswer(pCallInfo->hCall, 
                           // First call takes focus (mic and speaker), then
                           // the conference holds the focus after that.
                           !gbConf || g_conf == SIPX_CONF_NULL);
            break;

        case CALLSTATE_CONNECTED:

            if(gbConf)
            {
                // First call becomes the conference host
                if(g_conf == SIPX_CONF_NULL)
                {
                    SIPX_RESULT result = 
                        sipxConferenceCreate(g_hInst, &g_conf);
                    if(result == SIPX_RESULT_SUCCESS)
                    {
                        printf("Conference: %d created\n", (int)g_conf);
                    }
                    else                    {
                        printf("Error: sipxConferenceCreate returned: %d\n",
                               result);
                    }

                    result = 
                        sipxConferenceJoin(g_conf, pCallInfo->hCall);
                    if(result == SIPX_RESULT_SUCCESS)
                    {
                        printf("Initial call: %d joined to conference: %d\n", 
                               pCallInfo->hCall,
                               (int)g_conf);
                    }
                    else                    {
                        printf("Error: sipxConferenceJoin(call=%d, conf=%d) returned: %d\n",
                               pCallInfo->hCall,
                               (int)g_conf,
                               result);
                    }

                    SIPX_CONF callConf = SIPX_CONF_NULL;

                    result =
                         sipxCallGetConference(pCallInfo->hCall, callConf);
                    if(result == SIPX_RESULT_SUCCESS)
                    {
                    }
                    else if(g_conf != callConf)
                    {
                        printf("Call: %d not joined to conference: %d (%d)\n",
                               pCallInfo->hCall,
                               (int)g_conf,
                               (int)callConf);
                    }
                    else
                    {
                        printf("Error sipxCallGetConference unable to get initial conference from call: %d return: %d\n",
                               pCallInfo->hCall,
                               result);
                    }
                }

                // Subsequent calls get joined to first call/conference
                else
                {
                    // Conference has focus, subsequent calls will not get focus and will
                    // be in BRIDGED state.  So we will not get here.  In the bridged state
                    // put call on hold so that it can be joined to the conference
                    // Calls must be on hold to be joined to a conference.  So we must:
                    //   1) Hold the call
                    //   2) Join it to the conferences
                    //   3) Unhold the call
                }

                SIPX_CALL callHandleArray[10];
                int arraySize = sizeof(callHandleArray)/sizeof(SIPX_CALL);
                size_t numCalls = 0;
                for(int callIndex = 0; callIndex < arraySize; callIndex++)
                {
                    callHandleArray[callIndex] = pCallInfo->hCall;
                }

                SIPX_RESULT result =
                    sipxConferenceGetCalls(g_conf, callHandleArray, arraySize, numCalls);

                if(result != SIPX_RESULT_SUCCESS)
                {
                    printf("Error: sipxConferenceGetCalls(%d, ...) returned: %d\n",
                           (int)g_conf,
                           result);
                }
                else if(numCalls < 0)
                {
                    printf("Error: invalid number of calls (%d) in conference: %d\n",
                           (int)numCalls,
                           (int)g_conf);
                }
                else
                {
                    bool callAlreadyJoined = false;
                    for(size_t callIndex = 0; callIndex < numCalls; callIndex++)
                    {
                        if(callHandleArray[callIndex] == pCallInfo->hCall)
                        {
                            callAlreadyJoined = true;
                            break;
                        }
                    }

                    if(!callAlreadyJoined)
                    {
                        // Stop RTP stream so we can easily join it to conference
                        result = sipxCallHold(pCallInfo->hCall, true);
                        if(result == SIPX_RESULT_SUCCESS)
                        {
                            printf("Putting call: %d on hold to join with conference: %d\n",
                                   pCallInfo->hCall,
                                   (int)g_conf);
                        }
                        else 
                        {
                            printf("Error: sipxCallHold(%d, true) returned: %d\n",
                                   pCallInfo->hCall,
                                   (int)g_conf);
                        }
                    }
                }
                
            }

            SLEEP(1000) ;   // BAD: Do not block the callback thread

            // Start recording if option provided
            if(g_szRecordFile && ! g_recordingFile)
            {
                SIPX_RESULT recReturn =
                    recordFile(g_szRecordFile, pCallInfo->hCall);
                if(recReturn != SIPX_RESULT_SUCCESS)
                {
                    printf("Failed to start recording to file: %s return: %d\n",
                           g_szRecordFile,
                           recReturn);
                }
                else
                {
                    g_recordingFile = true;
                }
            }

            // Play file if provided
            if (g_szFile)
            {
                if (!playFile(g_szFile, pCallInfo->hCall))
                {
                    printf("Failed to play file: %s\n", g_szFile) ;
                }
            }

            // Play tones if provided
            if (g_szPlayTones)
            {
                // Do not block sipXtapi callback to play tones.
                // Set flag to start and play in different thread context
                g_playTonesNow = pCallInfo->hCall;
            }
            break;

        // Call is CONNECTED, but not in focus (mic and speaker are not mixed in)
        // Once, iniitial call and conference are created, all subsequent calls will
        // not get focus (hense BRIDGED when answered).
        case CALLSTATE_BRIDGED:
            if(gbConf)
            {
                SIPX_CALL callHandleArray[10];
                int arraySize = sizeof(callHandleArray)/sizeof(SIPX_CALL);
                size_t numCalls = 0;
                for(int callIndex = 0; callIndex < arraySize; callIndex++)
                {
                    callHandleArray[callIndex] = pCallInfo->hCall;
                }

                // Get call IDs for calls already in the conference
                SIPX_RESULT result =
                    sipxConferenceGetCalls(g_conf, callHandleArray, arraySize, numCalls);

                if(result != SIPX_RESULT_SUCCESS)
                {
                    printf("Error: sipxConferenceGetCalls(%d, ...) returned: %d\n",
                           (int)g_conf,
                           result);
                }
                else if(numCalls < 0)
                {
                    printf("Error: invalid number of calls (%d) in conference: %d\n",
                           (int)numCalls,
                           (int)g_conf);
                }
                else
                {
                    // Check if this call is already joined
                    bool callAlreadyJoined = false;
                    for(size_t callIndex = 0; callIndex < numCalls; callIndex++)
                    {
                        if(callHandleArray[callIndex] == pCallInfo->hCall)
                        {
                            callAlreadyJoined = true;
                            break;
                        }
                    }

                    // This call is not already joined to the conference
                    if(!callAlreadyJoined)
                    {
                        // Stop RTP stream so we can easily join it to conference
                        result = sipxCallHold(pCallInfo->hCall, true);
                        if(result == SIPX_RESULT_SUCCESS)
                        {
                            printf("Putting call: %d on hold to join with conference: %d\n",
                                   pCallInfo->hCall,
                                   (int)g_conf);
                        }
                        else 
                        {
                            printf("Error: sipxCallHold(%d, true) returned: %d\n",
                                   pCallInfo->hCall,
                                   (int)g_conf);
                        }
                    }
                }
                
            }
            break;

        case CALLSTATE_HELD:
            // This is a new incoming call held, so that it can be joined to the conferece
            if(gbConf)
            {
                SIPX_CALL callHandleArray[10];
                int arraySize = sizeof(callHandleArray)/sizeof(SIPX_CALL);
                size_t numCalls = 0;
                for(int callIndex = 0; callIndex < arraySize; callIndex++)
                {
                    callHandleArray[callIndex] = pCallInfo->hCall;
                }

                // Get list of calls already in the conference
                SIPX_RESULT result =
                    sipxConferenceGetCalls(g_conf, callHandleArray, arraySize, numCalls);

                if(result != SIPX_RESULT_SUCCESS)
                {
                    printf("Error: sipxConferenceGetCalls(%d, ...) returned: %d\n",
                           (int)g_conf,
                           result);
                }
                else if(numCalls < 0)
                {
                    printf("Error: invalid number of calls (%d) in conference: %d\n",
                           (int)numCalls,
                           (int)g_conf);
                }
                else
                {
                    // Check that call is not already in the conference
                    bool callAlreadyJoined = false;
                    for(size_t callIndex = 0; callIndex < numCalls; callIndex++)
                    {
                        if(callHandleArray[callIndex] == pCallInfo->hCall)
                        {
                            callAlreadyJoined = true;
                            break;
                        }
                    }

                    if(!callAlreadyJoined)
                    {
                        // Join the call to the conference
                        result = 
                            sipxConferenceJoin(g_conf, pCallInfo->hCall);
                        if(result == SIPX_RESULT_SUCCESS)
                        {
                            printf("Joining call: %d to %d existing calls in conference: %d\n",
                                   pCallInfo->hCall,
                                   (int)numCalls,
                                   (int)g_conf);
                        }
                        else
                        {
                            printf("Error: sipxConferenceJoin(conf=%d, call=%d) returned: %d\n",
                                   (int)g_conf,
                                   pCallInfo->hCall,
                                   result);
                        }

                        // Unhold this call so audio streams and gets mixed into conference
                        result = 
                            sipxCallUnhold(pCallInfo->hCall);
                        if(result == SIPX_RESULT_SUCCESS)
                        {
                            printf("Taking call: %d off hold after joining to conference: %d\n",
                                   pCallInfo->hCall,
                                   (int)g_conf);
                        }
                        else
                        {
                            printf("Error: sipxCallUnhold(%d) returned: %d\n",
                                   pCallInfo->hCall,
                                   result);
                        }
                    }
                }
            }
            break;

        case CALLSTATE_REMOTE_HELD:
            break;

        case CALLSTATE_DISCONNECTED:
            // Stop recording if option provided
            if(g_szRecordFile)
            {
                size_t remainingLegs = 0;
                SIPX_CALL callHandles[MAX_CONFERENCE_LEGS];
                if(g_conf != SIPX_CONF_NULL)
                {
                    SIPX_RESULT callCountReturn = 
                        sipxConferenceGetCalls(g_conf,
                                               callHandles,
                                               MAX_CONFERENCE_LEGS,
                                               remainingLegs);
                    if(callCountReturn != SIPX_RESULT_SUCCESS)
                    {
                        printf("CALLSTATE_DISCONNECTED hCall=%d sipxConferenceGetCalls(%d) returned: %d",
                               pCallInfo->hCall,
                               (int)g_conf,
                               callCountReturn);
                        remainingLegs = 0;
                    }
                    else
                    {
                        UtlString callHandleString;
                        for(size_t callIndex = 0; callIndex < remainingLegs; callIndex++)
                        {
                            callHandleString.appendFormat("%s%d",
                                                          callIndex ? ", " : "",
                                                          callHandles[callIndex]);
                        }
                                                          
                        printf("CALLSTATE_DISCONNECTED hCall=%d %d calls [%s] remain in conference", 
                               pCallInfo->hCall,
                               (int)remainingLegs,
                               callHandleString.data());
                    }
                }

                // If we have a conference and this is the last leg
                // disconnecting, then we stop the recording
                if(remainingLegs < 2)
                {
                    SIPX_RESULT recReturn =
                        sipxCallAudioRecordFileStop(pCallInfo->hCall);
                    if(recReturn != SIPX_RESULT_SUCCESS)
                    {
                        printf("Failed to stop recording to file: %s return: %d\n",
                               g_szRecordFile,
                               recReturn);
                        if(remainingLegs > 0)
                        {
                            recReturn = sipxCallAudioRecordFileStop(callHandles[0]);
                            if(recReturn != SIPX_RESULT_SUCCESS)
                            {
                                printf("Failed to stop recording to file: %s hCall: %d return: %d\n",
                                       g_szRecordFile,
                                       callHandles[0],
                                       recReturn);
                            }
                        }
                    }
                    g_recordingFile = false;
                }
            }

            sipxCallDestroy(pCallInfo->hCall) ;
            break;

        case CALLSTATE_DESTROYED:
        {
            if (gbOneCallMode)
            {
               gbShutdown = true;
            }
            size_t remainingLegs = 0;
            SIPX_CALL callHandles[MAX_CONFERENCE_LEGS];
            if(g_conf != SIPX_CONF_NULL)
            {
                SIPX_RESULT callCountReturn = 
                    sipxConferenceGetCalls(g_conf,
                                           callHandles,
                                           MAX_CONFERENCE_LEGS,
                                           remainingLegs);
                if(callCountReturn != SIPX_RESULT_SUCCESS)
                {
                    printf("CALLSTATE_DESTROYED hCall=%d sipxConferenceGetCalls(%d) returned: %d",
                           pCallInfo->hCall,
                           (int)g_conf,
                           callCountReturn);
                    remainingLegs = 0;
                }
                else
                {
                    UtlString callHandleString;
                    for(size_t callIndex = 0; callIndex < remainingLegs; callIndex++)
                    {
                        callHandleString.appendFormat("%s%d",
                                                      callIndex ? ", " : "",
                                                      callHandles[callIndex]);
                    }
                                                      
                    printf("CALLSTATE_DESTROYED hCall=%d %d calls [%s] remain in conference", 
                           pCallInfo->hCall,
                           (int)remainingLegs,
                           callHandleString.data());
                }

                // If we have a conference and this is the last leg
                // disconnecting, then we stop the recording
                if(remainingLegs < 2)
                {
                    SIPX_RESULT recReturn =
                        sipxCallAudioRecordFileStop(pCallInfo->hCall);
                    if(recReturn != SIPX_RESULT_SUCCESS)
                    {
                        printf("Failed to stop recording to file: %s return: %d\n",
                               g_szRecordFile,
                               recReturn);
                        if(remainingLegs > 0)
                        {
                            recReturn = sipxCallAudioRecordFileStop(callHandles[0]);
                            if(recReturn != SIPX_RESULT_SUCCESS)
                            {
                                printf("Failed to stop recording to file: %s hCall: %d return: %d\n",
                                       g_szRecordFile,
                                       callHandles[0],
                                       recReturn);
                            }
                        }
                    }
                    g_recordingFile = false;
                }

                // Never destroy conference
                // Uncomment other if statement if you want to kill conference 
                // after call count goes back down to zero
                if(0) 
                // if(remainingLegs == 0)
                {
                    SIPX_RESULT confDestResult = sipxConferenceDestroy(g_conf);
                    if(confDestResult != SIPX_RESULT_SUCCESS)
                    {
                        printf("sipxConferenceDestroy(%d) FAILED, return: %d",
                               (int)g_conf,
                               confDestResult);
                    }
                }

            }
        }
            break;

        default:
            break;
        }
    }
    else if (category == EVENT_CATEGORY_MEDIA)
    {
       SIPX_MEDIA_INFO* pMediaInfo = static_cast<SIPX_MEDIA_INFO*>(pInfo);

       switch(pMediaInfo->event)
       {
       case MEDIA_LOCAL_START:
           printf("* Negotiated codec: %s, payload type %d\n",
                  pMediaInfo->codec.audioCodec.cName, pMediaInfo->codec.audioCodec.iPayloadType);
       	  break;

       case MEDIA_MIC_ENERGY_LEVEL:
       case MEDIA_SPEAKER_ENERGY_LEVEL:
           {
               const char* label = "";
               if(pMediaInfo->event == MEDIA_MIC_ENERGY_LEVEL)
               {
                   label = "Mic";
               }
               else if(pMediaInfo->event == MEDIA_SPEAKER_ENERGY_LEVEL)
               {
                   label = "Speaker";
               }
               int level = 0;
               if(pMediaInfo->idleTime > 0)
               {
                   level = (int)(log10((double)pMediaInfo->idleTime) * 1.8) - 8;
               }
               if(level < 0) level = 0;
               if(level > 8) level = 8;
               //printf("%s: %d\n", label, pMediaInfo->idleTime);
               printf("%s: %.*s%.*s\n",
                      label,
                      level,
                      "=====***",
                      8 - level,
                      "________");
           }
           break;
       default:
           break;
       }
    }
    printf("event done\n");
    return true;
}


SIPX_LINE lineInit(SIPX_INST hInst, char* szIdentity, char* szUsername, char* szPassword, char* szRealm)
{
    SIPX_LINE hLine = SIPX_LINE_NULL ;

    if (szIdentity && strlen(szIdentity))
    {
        sipxLineAdd(hInst, szIdentity, &hLine) ;

        if (    szUsername && strlen(szUsername) &&
            szPassword && strlen(szPassword) &&
            szRealm && strlen(szRealm))
        {
            sipxLineAddCredential(hLine, szUsername, szPassword, szRealm) ;
            sipxLineRegister(hLine, true);
        }
    }
    else
    {
        sipxLineAdd(hInst, "sip:receivecall@localhost", &hLine) ;
    }

    return hLine ;
}

void dumpLocalContacts(SIPX_INST hInst)
{
    SIPX_CONTACT_ADDRESS contacts[10] ;
    size_t nContacts;

    SIPX_RESULT status = sipxConfigGetLocalContacts(hInst, contacts, 10, nContacts) ;
    if (status == SIPX_RESULT_SUCCESS)
    {
        for (size_t i = 0; i<nContacts; i++)
        {
            const char* szType = "UNKNOWN" ;
            switch (contacts[i].eContactType)
            {
                case CONTACT_LOCAL:
                    szType = "LOCAL" ;
                    break ;
                case CONTACT_NAT_MAPPED:
                    szType = "NAT_MAPPED" ;
                    break ;
                case CONTACT_RELAY:
                    szType = "RELAY" ;
                    break ;
                case CONTACT_CONFIG:
                    szType = "CONFIG" ;
                    break ;
                case CONTACT_AUTO:
                    szType = "AUTO";
                    break;
                case CONTACT_ALL:
                    szType = "ALL";
                    break;
            }
            printf("<-> Type %s, Interface: %s, Ip %s, Port %d\n",
                    szType, contacts[i].cInterface, contacts[i].cIpAddress,
                    contacts[i].iPort) ;
        }
    }
    else
    {
        printf("<-> Unable to query local contact addresses\n") ;
    }
}


int local_main(int argc, char* argv[])
{
    bool bError = true ;
    int iDuration, iSipPort, iRtpPort ;
    char* szBindAddr;
    char* szOutDevice;
    char* szInDevice;
    bool bLoopback ;
    bool bAEC = false;
    char* szIdentity ;
    char* szUsername ;
    char* szPassword ;
    char* szRealm ;
    int refreshPeriod;
    int deviceRate;
    int mediaRate;
    char* szStunServer ;
    char* szProxy ;
    SIPX_INST hInst ;
    SIPX_LINE hLine ;

    if ( signal( SIGINT, ctrlCHandler ) == SIG_ERR )
    {
       printf("Couldn't install signal handler for SIGINT\n");
       exit(1);
    }

    if ( signal( SIGTERM, ctrlCHandler ) == SIG_ERR )
    {
       printf("Couldn't install signal handler for SIGTERM\n");
       exit(1);
    }

    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &szBindAddr,
                   &szInDevice, &szOutDevice,
                  &g_szPlayTones, &g_szFile, &bLoopback, &gbOneCallMode,
                  &szIdentity, &szUsername, &szPassword, &szRealm, &refreshPeriod,
                  &deviceRate, &mediaRate,
                  &szStunServer, &szProxy, &g_szRecordFile) &&
        (iDuration > 0) && (portIsValid(iSipPort)) && (portIsValid(iRtpPort)))
    {
        if (bLoopback)
        {
            initLoopback() ;
        }

        // For testing reasons it is convenient to put the codec plugins in the current directory.
        // This is not generally a safe thing to do in production.
        //UtlString cwd(".");
        //CpMediaInterfaceFactoryImpl::addCodecPaths(1, &cwd);
        printf("Attempting to use audio device sample rate: %d samples/sec.\n", deviceRate);
        printf("Using media/flowgraph frame sample rate: %d samples/sec.\n", mediaRate);

        // Initialize sipX TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("ReceiveCall.log");
        if (sipxInitialize(&hInst, 
                           iSipPort, 
                           iSipPort, 
                           5061, 
                           iRtpPort, 
                           16, 
                           szIdentity, 
                           szBindAddr,
                           false, // use sequential RTP/RTCP ports
                           NULL, // cert. nickname
                           NULL, // cert. password
                           NULL, // DB location
                           true, // Enable local audio
                           mediaRate, // Flowgraph internal sample rate
                           deviceRate, // Audio device sample rate
                           10, // internal media frame size (milliseconds)
                           szInDevice ? szInDevice : "", // Audio input device
                           szOutDevice ? szOutDevice : ""  // Audio output device
           ) == SIPX_RESULT_SUCCESS)
        {            
            g_hInst = hInst;
            if (szProxy)
            {
                sipxConfigSetOutboundProxy(hInst, szProxy);
            }

            sipxConfigEnableRport(hInst, true);
            // Don't use Rport mapping discovered for future contact headers
            sipxConfigEnableRportMapping(g_hInst, false);

            if (szStunServer)
            {
                sipxConfigEnableStun(hInst, szStunServer, DEFAULT_STUN_PORT, 28) ;
            }

            // Set in sipxInitialize.  Keeping this for testing device change after initialization
            if (0) // (szOutDevice)
            {
                SIPX_RESULT sipxStatus = sipxAudioSetCallOutputDevice(g_hInst, szOutDevice);
                if (sipxStatus != SIPX_RESULT_SUCCESS)
                {
                    printf("!! Setting output device %s failed: %d !!\n", szOutDevice, sipxStatus);
                }
                else
                {
                    printf("Successfully set output device to: %s\n", szOutDevice);
                }
            }

            // Set in sipxInitialize.  Keeping this for testing device change after initialization
            if (0) //(szInDevice)
            {
                SIPX_RESULT sipxStatus = sipxAudioSetCallInputDevice(g_hInst, szInDevice);
                if (sipxStatus != SIPX_RESULT_SUCCESS)
                { 
                    printf("!! Setting input device %s failed: %d !!\n", szOutDevice, sipxStatus);
                }
                else
                {
                    printf("Successfully set input device to: %s\n", szInDevice);
                }

            }
            sipxEventListenerAdd(hInst, EventCallBack, NULL) ;
                
            // Enable/disable AEC.
            if (bAEC)
            {
                sipxAudioSetAECMode(g_hInst, SIPX_AEC_CANCEL_AUTO);
            }
            else
            {
                sipxAudioSetAECMode(g_hInst, SIPX_AEC_DISABLED);
            }

#if defined(_WIN32) && defined(VIDEO)
            if (bVideo)
            {
                gPreviewDisplay.type = SIPX_WINDOW_HANDLE_TYPE;
                gPreviewDisplay.handle = ghPreview;
                sipxConfigSetVideoPreviewDisplay(hInst, &gPreviewDisplay);
            }
#endif

            // get first contact
            size_t numAddresses = 0;
            SIPX_CONTACT_ADDRESS address;
            sipxConfigGetLocalContacts(hInst, 
                                    &address,
                                    1,
                                    numAddresses);

            if (bUseCustomTransportReliable)
            {
                sipxConfigExternalTransportAdd(hInst,
                                            ghTransport,
                                            true,
                                            "tribble",
                                            address.cIpAddress,
                                            -1,                                            
                                            tribbleProc,
                                            "tribble");
                startTribbleListener(address.cIpAddress);
            }
            
            if (bUseCustomTransportUnreliable)
            {
                startFlibbleListener(address.cIpAddress);
                sipxConfigExternalTransportAdd(hInst,
                                            ghTransport,
                                            false,
                                            "flibble",
                                            address.cIpAddress,
                                            -1,                                            
                                            flibbleProc,
                                            address.cIpAddress);
                                                       
                gContactId = lookupContactId(address.cIpAddress, "flibble", ghTransport);
            }

            if (refreshPeriod > 0)
            {
                sipxConfigSetRegisterExpiration(hInst, refreshPeriod);
            }

            hLine = lineInit(hInst, szIdentity, szUsername, szPassword, szRealm) ;
            printf("Line: %d initialized\n", hLine);

            dumpLocalContacts(hInst) ;

            while (!gbShutdown)
            {
                SLEEP(100);
                if(g_playTonesNow != SIPX_CALL_NULL)
                {
                    if (!playTones(g_szPlayTones, g_playTonesNow ))
                    {
                        printf("Failed to play tones: %s\n", g_szPlayTones) ;
                    }
                    g_playTonesNow = SIPX_CALL_NULL;
                }
            }

            sipxUnInitialize(hInst, true);
        }
        else
        {
            printf("unable to initialize sipXtapi layer\n") ;
        }
    }
    else
    {
        usage(argv[0]) ;
    }

#if defined(_WIN32) && defined(VIDEO)
    PostMessage(hMain, WM_CLOSE, 0, 0L);
#endif
    return (int) bError ;
}

int main(int argc, char* argv[])
{
#if defined(_WIN32) && defined(VIDEO)
    CreateWindows();

    DWORD dwThreadId = 0;
    CmdParams cmdParams;
    cmdParams.argc = argc;
    cmdParams.argv = argv;

    HANDLE hThread = CreateThread(NULL, 0, ConsoleStart, &cmdParams, 0, &dwThreadId);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // Ask for sipXtapi shutdown and wait for its completion.
    ctrlCHandler(0);
    WaitForSingleObject(hThread, INFINITE);
    return 0;
#else
    return local_main(argc, argv);
#endif
}

#if defined(_WIN32) && defined(VIDEO)
DWORD WINAPI ConsoleStart(LPVOID lpParameter)
{
    CmdParams* pParams = (CmdParams*)lpParameter;
	local_main(pParams->argc, pParams->argv);

    return 0;
}
#endif

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */

void startTribbleListener(const char* szIp)
{
    
}

void startFlibbleListener(const char* szIp)
{
    gpFlibbleTask = new  FlibbleTask(szIp);
    gpFlibbleTask->start();
}
