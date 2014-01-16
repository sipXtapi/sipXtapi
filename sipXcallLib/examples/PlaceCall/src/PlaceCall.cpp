//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#include "PlaceCall.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#  include "PlaceCallWntApp.h"
DWORD WINAPI ConsoleStart(LPVOID lpParameter);
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "ExternalTransport.h"

#define MAX_RECORD_EVENTS       16
#define portIsValid(p) ((p) >= 1 && (p) <= 65535)

SIPX_INST g_hInst = NULL ;      // Handle to the sipXtapi instance
SIPX_LINE g_hLine = 0 ;         // Line Instance (id, auth, etc)
SIPX_CALL g_hCall = 0 ;         // Handle to a call
bool bUseCustomTransportReliable = false;
bool bUseCustomTransportUnreliable = false;
SIPX_TRANSPORT ghTransport = SIPX_TRANSPORT_NULL;

SIPX_CALLSTATE_EVENT    g_eRecordEvents[MAX_RECORD_EVENTS] ;    // List of last N events
int                     g_iNextEvent ;      // Index for g_eRecordEvents ringer buffer
#if defined(_WIN32) && defined(VIDEO)
SIPX_VIDEO_DISPLAY gDisplay;
SIPX_VIDEO_DISPLAY gPreviewDisplay;
extern HWND ghPreview;
extern HWND ghVideo;
extern HWND hMain;
static bool bVideo = false;
#endif


bool shutdownCall();
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
   shutdownCall();
}

// Print usage message
void usage(const char* szExecutable)
{
    char szBuffer[64];

    sipxConfigGetVersion(szBuffer, 64);
    printf("\nUsage:\n") ;
    printf("   %s <options> [URL]\n", szExecutable) ;
    printf("      using %s\n", szBuffer);
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -d <int> durationInSeconds (default=30 seconds)\n") ;
    printf("   -t <string> play tones (0123456789#!*,) (default = none)\n") ;
    printf("   -f <string> play file (default = none)\n") ;
    printf("   -b play file from buffer (default = none)\n") ;
    printf("   -p <int> SIP port (default = 5060)\n") ;
    printf("   -r <int> RTP port start (default = 9000)\n") ;
    printf("   -R use rport as part of via (disabled by default)\n") ;
    printf("   -B <string> ip address to bind to\n");
    printf("   -u <string> username (for authentication)\n") ;
    printf("   -a <string> password  (for authentication)\n") ;
    printf("   -m <string> realm  (for authentication)\n") ;
    printf("   -i <string> from identity\n") ;
    printf("   -S <string> stun server\n") ;
    printf("   -x <string> proxy (outbound proxy)\n");
    printf("   -v show sipXtapi version\n");
    printf("   -c <int> repeat count/Prank mode (call end point N times)\n") ;
    printf("   -I <string> call input device name\n");
    printf("   -O <string> call output device name\n");
    printf("   -g <float> set input device gain (0.001-10.0) (default = 1.0)n");
    printf("   -volume <int> set output volume (1-100) (default = 70)\n");
    printf("   -C <string> codec name (default all available)\n");
    printf("   -L list all supported codecs\n");
    printf("   -aec enable acoustic echo cancelation\n");
    printf("   -agc enable automatic gain control\n");
    printf("   -denoise enable speex denoiser\n");
    printf("   -E use bogus custom external transport, reliable (transport=tribble)\n");
    printf("   -e use bogus custom external transport, unreliable (transport=flibble)\n");
#if defined(_WIN32) && defined(VIDEO)
    printf("   -V place a video call\n");
#endif
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char*  argv[],
               int*   pDuration,
               int*   pSipPort,
               int*   pRtpPort,
               char** pszPlayTones,
               char** pszFile,
               char** pszFileBuffer,
               char** pszUrl,
               bool*  bUseRport,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszFromIdentity,
               char** pszStunServer,
               char** pszProxy,
               char** pszBindAddress,
               int*   pRepeatCount,
               char** pszInputDevice,
               char** pszOutputDevice,
               float*   inputGain,
               int*   outputVolume,
               char** pszCodecName,
               bool*  bCodecList,
               bool*  bAEC,
               bool*  bAGC,
               bool*  bDenoise,
               bool*  bUseCustomTransportReliable,
               bool*  bUseCustomTransportUnreliable)
{
    bool bRC = false ;
    char szBuffer[64];

    assert(pDuration && pszPlayTones && pszUrl) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pRepeatCount = 1 ;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *pszFileBuffer = NULL ;
    *pszUrl = NULL ;
    *bUseRport = false ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszFromIdentity = NULL ;
    *pszStunServer = NULL ;
    *pszProxy = NULL;
    *pszBindAddress = NULL;
    *pszInputDevice = NULL;
    *pszOutputDevice = NULL;
    *inputGain = -1.0;
    *outputVolume = -1;
    *pszCodecName = NULL;
    *bCodecList = false;
    *bAEC = false;
    *bAGC = false;
    *bDenoise = false;

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
        else if (strcmp(argv[i], "-b") == 0)
        {
            if ((i+1) < argc)
            {
                *pszFileBuffer = strdup(argv[++i]) ;
            }
            else
            {
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
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if ((i+1) < argc)
            {
                *pszFromIdentity = strdup(argv[++i]) ;
            }
            else
            {
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
        else if (strcmp(argv[i], "-S") == 0)
        {
            if ((i+1) < argc)
            {
                *pszStunServer = strdup(argv[++i]) ;
            }
            else
            {
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
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-R") == 0)
        {
            *bUseRport = true ;
        }
        else if (strcmp(argv[i], "-L") == 0)
        {
            *bCodecList = true ;
            bRC = true ;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            sipxConfigGetVersion(szBuffer, 64);
            printf("%s\n", szBuffer);
            exit(0);
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            if ((i+1) < argc)
            {
                *pRepeatCount = atoi(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
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
        else if (strcmp(argv[i], "-g") == 0)
        {
            if ((i+1) < argc)
            {
                *inputGain = atof(argv[++i]);
            }
            else
            {
                break; // Error
            }
        }
        else if (strcmp(argv[i], "-volume") == 0)
        {
            if ((i+1) < argc)
            {
                *outputVolume = atoi(argv[++i]);
            }
            else
            {
                break; // Error
            }
        }
        else if (strcmp(argv[i], "-C") == 0)
        {
            if ((i+1) < argc)
            {
                *pszCodecName = strdup(argv[++i]) ;
            }
            else
            {
                break ; // Error
            }
        }
#ifdef VIDEO
        else if (strcmp(argv[i], "-V") == 0)
        {
            bVideo = true;
        }
#endif
        else if (strcmp(argv[i], "-aec") == 0)
        {
            *bAEC = true;
        }
        else if (strcmp(argv[i], "-agc") == 0)
        {
            *bAGC = true;
        }
        else if (strcmp(argv[i], "-denoise") == 0)
        {
            *bDenoise = true;
        }
        else if (strcmp(argv[i], "-E") == 0)
        {
            *bUseCustomTransportReliable = true;            
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            *bUseCustomTransportUnreliable = true;            
        }
        else
        {
            if ((i+1) == argc)
            {
                *pszUrl = strdup(argv[i]) ;
                bRC = true ;
            }
            else
            {
                break ; // Error
            }
        }
    }
    return bRC ;
}

bool EventCallBack(SIPX_EVENT_CATEGORY category, 
                   void* pInfo, 
                   void* pUserData)
{
    char cBuf[1024] ;
    printf("%s\n", sipxEventToString(category, pInfo, cBuf, sizeof(cBuf))) ;    

    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = static_cast<SIPX_CALLSTATE_INFO*>(pInfo);
        printf("    hCall=%d, hAssociatedCall=%d\n", pCallInfo->hCall, pCallInfo->hAssociatedCall) ;

        g_eRecordEvents[g_iNextEvent] = pCallInfo->event;
        g_iNextEvent = (g_iNextEvent + 1) % MAX_RECORD_EVENTS ;
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
       }
    }
    return true;
}

// Wait for the designated event for at worst ~iTimeoutInSecs seconds
bool WaitForSipXEvent(SIPX_CALLSTATE_EVENT event, int iTimeoutInSecs)
{
    bool bFound = false ;
    int  tries = 0;

    // Kids, don't try this at home -- This method of waiting for events is
    // not recommended.  Generally, most UAs are asynchronous and event
    // driven -- if you need to want for an event, build something that is
    // thread safe and doesn't use sleeps.
    while (!bFound)
    {
        for (int i=0;i<MAX_RECORD_EVENTS; i++)
        {
            if (g_eRecordEvents[i] == event)
            {
                bFound = true ;
                break ;
            }
        }

        if (!bFound)
        {
            SLEEP(1000) ;
            if (++tries > (iTimeoutInSecs))
            {
                break ;
            }
#ifdef ITERATE_DEVICES
            // Change to a different input and output device every 10 seconds
            if(tries % 10 == 0)
            {
                size_t numInputs = 0;
                size_t numOutputs = 0;
                sipxAudioGetNumInputDevices(g_hInst, numInputs);
                sipxAudioGetNumOutputDevices(g_hInst, numOutputs);

                int deviceIndex = tries / 10;

                const char* outputDeviceString;
                sipxAudioGetOutputDevice(g_hInst, deviceIndex % numOutputs, outputDeviceString);
                printf("Switching speaker device to: \"%s\"\n", outputDeviceString);
                sipxAudioSetCallOutputDevice(g_hInst, outputDeviceString);

                const char* inputDeviceString;
                sipxAudioGetInputDevice(g_hInst, deviceIndex % numInputs, inputDeviceString);
                printf("Switching mic device to: \"%s\"\n", inputDeviceString);
                sipxAudioSetCallInputDevice(g_hInst, inputDeviceString);
            }
#endif
        }
    }

    return bFound ;
}

// Clear the event log
void ClearSipXEvents()
{
    for (int i=0;i<MAX_RECORD_EVENTS; i++)
    {
        g_eRecordEvents[i] = CALLSTATE_UNKNOWN ;
    }
    g_iNextEvent = 0 ;
}


void dumpLocalContacts(SIPX_CALL hCall)
{
    SIPX_CONTACT_ADDRESS contacts[10] ;
    size_t nContacts;

    SIPX_RESULT status = sipxConfigGetLocalContacts(g_hInst, contacts, 10, nContacts) ;
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
                default:
                    assert(false) ;
                    break ;
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


// Place a call to szSipUrl as szFromIdentity
bool placeCall(char* szSipUrl, char* szFromIdentity, char* szUsername, char* szPassword, char *szRealm)
{
    bool bRC = false ;

    if ((szFromIdentity == NULL) || strlen(szFromIdentity) == 0)
    {
        szFromIdentity = "\"PlaceCall Demo\" <sip:placecalldemo@localhost>" ;
    }

    printf("<-> Placing call to \"%s\" as \"%s\"\n", szSipUrl, szFromIdentity) ;
    printf("<-> Username: %s, passwd: %s, realm: %s (all required for auth)\n", szUsername, szPassword, szRealm) ;

    sipxLineAdd(g_hInst, szFromIdentity, &g_hLine) ;
    if (szUsername && szPassword && szRealm)
    {
        sipxLineAddCredential(g_hLine, szUsername, szPassword, szRealm) ;
    }
#if defined(_WIN32) && defined(VIDEO)
    if (bVideo)
    {
        gPreviewDisplay.type = SIPX_WINDOW_HANDLE_TYPE;
        gPreviewDisplay.handle = ghPreview;
        sipxConfigSetVideoPreviewDisplay(g_hInst, &gPreviewDisplay);
    }
#endif

    sipxCallCreate(g_hInst, g_hLine, &g_hCall) ;
    dumpLocalContacts(g_hCall) ;

    // get first contact
    size_t numAddresses = 0;
    SIPX_CONTACT_ADDRESS address;
    sipxConfigGetLocalContacts(g_hInst, 
                               &address,
                               1,
                               numAddresses);
    


    if (bUseCustomTransportReliable)
    {
        sipxConfigExternalTransportAdd(g_hInst,
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
        sipxConfigExternalTransportAdd(g_hInst,
                                       ghTransport,
                                       true,
                                       "flibble",
                                       address.cIpAddress,
                                       -1,
                                       flibbleProc,
                                       address.cIpAddress);
                                               
        gContactId = lookupContactId(address.cIpAddress, "flibble", ghTransport);
    }

#if defined(_WIN32) && defined(VIDEO)
    if (bVideo)
    {
        gDisplay.type = SIPX_WINDOW_HANDLE_TYPE;
        gDisplay.handle = ghVideo;
        sipxCallConnect(g_hCall, szSipUrl, gContactId, &gDisplay, NULL);
    }
    else
#endif
    {
        sipxCallConnect(g_hCall, szSipUrl, gContactId);
    }
    bRC = WaitForSipXEvent(CALLSTATE_CONNECTED, 30) ;

    return bRC ;
}


// Drop call, clean up resources
bool shutdownCall()
{
    if (g_hCall != 0)
    {
        printf("<-> Shutting down Call\n") ;

        ClearSipXEvents() ;
        sipxCallDestroy(g_hCall) ;
        g_hCall = 0;
        sipxLineRemove(g_hLine) ;

        WaitForSipXEvent(CALLSTATE_DESTROYED, 5) ;
    }
    return true ;
}


// Play a series of tones
bool playTones(char* szPlayTones)
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
                if (sipxCallStartTone(g_hCall, (SIPX_TONE_ID) toneId, true, true) != SIPX_RESULT_SUCCESS)
                {
                    printf("Playtone returned error\n");
                }
                SLEEP(500) ;
                sipxCallStopTone(g_hCall) ;
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


// Play a file (8000 samples/sec, 16 bit unsigned, mono PCM)
bool playFile(char* szFile)
{
    bool bRC = true ;
    sipxCallAudioPlayFileStart(g_hCall, szFile, false, true, true) ;

    return bRC ;
}

// Play a file (8000 samples/sec, 16 bit unsigned, mono PCM)
bool playFileBuffer(char* szFile)
{
    bool bRC = false ;
    FILE* fp = fopen(szFile, "rb") ;
    if (fp)
    {
        fseek(fp, 0, SEEK_END) ;
        unsigned long length = ftell(fp) ;
        fseek(fp, 0, SEEK_SET) ;

        if (length > 0)
        {
            char* pBuf = (char*) malloc(length) ;
            if (pBuf)
            {
                fread(pBuf, 1, (size_t) length, fp) ;
                sipxCallPlayBufferStart(g_hCall, pBuf, (int) length, RAW_PCM_16, false, true, true) ;
                free(pBuf) ;
                bRC = true ;
            }
            else
            {
                printf("Unable to allocate memory for file buffer\n") ;
            }
        }
    }
    else
    {
        printf("Unable to open: %s\n", szFile) ;
    }

    return bRC ;
}


// Display the list of input & output devices
void dumpInputOutputDevices()
{
    size_t numDevices ;

    if (sipxAudioGetNumInputDevices(g_hInst, numDevices) == SIPX_RESULT_SUCCESS)
    {
        printf("Input Devices: %d\n", numDevices) ;
        for (size_t i=0; i<numDevices; i++)
        {
            const char* szDevice ;
            sipxAudioGetInputDevice(g_hInst, i, szDevice) ;
            printf("\t#%d: %s\n", i, szDevice) ;
        }
    }

    if (sipxAudioGetNumOutputDevices(g_hInst, numDevices) == SIPX_RESULT_SUCCESS)
    {
        printf("Output Devices: %d\n", numDevices) ;
        for (size_t i=0; i<numDevices; i++)
        {
            const char* szDevice ;
            sipxAudioGetOutputDevice(g_hInst, i, szDevice) ;
            printf("\t#%d: %s\n", i, szDevice) ;
        }
    }

    // sipxAudioSetCallOutputDevice(g_hInst, "NONE") ;
    // sipxAudioSetCallInputDevice(g_hInst, "SigmaTel Audio") ;
}


int local_main(int argc, char* argv[])
{
    bool bError = false ;
    int iDuration, iSipPort, iRtpPort, iRepeatCount ;
    char* szPlayTones;
    char* szSipUrl;
    char* szFile;
    char* szFileBuffer;
    char* szUsername;
    char* szPassword;
    char* szRealm;
    char* szFromIdentity;
    char* szStunServer;
    char* szProxy;
    char* szBindAddr;
    char* szOutDevice;
    char* szInDevice;
    float inputGain;
    int outputVolume;
    char* szCodec;
    bool bUseRport;
    bool bCList;
    bool bAEC;
    bool bAGC;
    bool bDenoise;

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
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &szPlayTones,
            &szFile, &szFileBuffer, &szSipUrl, &bUseRport, &szUsername, 
            &szPassword, &szRealm, &szFromIdentity, &szStunServer, &szProxy, 
            &szBindAddr, &iRepeatCount, &szInDevice, &szOutDevice, &inputGain, &outputVolume,
            &szCodec, &bCList,
            &bAEC, &bAGC, &bDenoise, &bUseCustomTransportReliable, &bUseCustomTransportUnreliable) 
            && (iDuration > 0) && (portIsValid(iSipPort)) && (portIsValid(iRtpPort)))
    {
        // initialize sipx TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("PlaceCall.log");
        int mediaEngineSampleRate = 16000;
        sipxInitialize(&g_hInst, 
                       iSipPort, 
                       iSipPort, 
                       -1, 
                       iRtpPort,
                       DEFAULT_CONNECTIONS, 
                       DEFAULT_IDENTITY, 
                       szBindAddr,
                       false, // use sequential RTP/RTCP ports
                       NULL, // cert. nickname
                       NULL, // cert. password
                       NULL, // DB location
                       true, // Enable local audio
                       mediaEngineSampleRate,
                       48000); // Audio device sample rate 
        sipxConfigEnableRport(g_hInst, bUseRport) ;
        if(bUseRport)
        {
            // Don't use Rport mapping discovered for future contact headers
            sipxConfigEnableRportMapping(g_hInst, false);
        }
        dumpInputOutputDevices() ;
        sipxEventListenerAdd(g_hInst, EventCallBack, NULL) ;

        // Enable/disable AEC.
        if (bAEC)
           sipxAudioSetAECMode(g_hInst, SIPX_AEC_CANCEL_AUTO) ;
        else
           sipxAudioSetAECMode(g_hInst, SIPX_AEC_DISABLED) ;

        // Enable/disable AGC
        sipxAudioSetAGCMode(g_hInst, bAGC);

        if (bDenoise)
           sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_HIGH);
        else
           sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_DISABLED);

        if (bCList)
        {
            int numAudioCodecs;
            int numVideoCodecs;
            int index;
            SIPX_AUDIO_CODEC audioCodec;
            SIPX_VIDEO_CODEC videoCodec;

            printf("Audio codecs:\n");
            if (sipxConfigGetNumAudioCodecs(g_hInst, &numAudioCodecs) == SIPX_RESULT_SUCCESS)
            {
                for (index=0; index<numAudioCodecs; ++index)
                {
                    if (sipxConfigGetAudioCodec(g_hInst, index, &audioCodec) == SIPX_RESULT_SUCCESS)
                    {
                        printf("  audio %02d : %s\n", index, audioCodec.cName);
                    }
                    else
                    {
                        printf("Error in retrieving audio codec #%d\n", index);
                    }
                }
            }
            else
            {
                printf("Error in retrieving number of audio codecs\n");
            }
#ifdef VIDEO
            printf("Video codecs:\n");
            if (sipxConfigGetNumVideoCodecs(g_hInst, &numVideoCodecs) == SIPX_RESULT_SUCCESS)
            {
                for (index=0; index<numVideoCodecs; ++index)
                {
                    if (sipxConfigGetVideoCodec(g_hInst, index, &videoCodec) == SIPX_RESULT_SUCCESS)
                    {
                        printf("  video %02d : %s\n", index, videoCodec.cName);
                    }
                    else
                    {
                        printf("Error in retrieving video codec #%d\n");
                    }
                }
            }
            else
            {
                printf("Error in retrieving number of video codecs\n");
            }
#endif // VIDEO            
            sipxUnInitialize(g_hInst, true);
            exit(0);
        }
        if (szProxy)
        {
            sipxConfigSetOutboundProxy(g_hInst, szProxy);
        }

        if (szStunServer)
        {
            sipxConfigEnableStun(g_hInst, szStunServer, DEFAULT_STUN_PORT, 28) ;
        }
        if (szOutDevice)
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
        if (szInDevice)
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
        if (szCodec)
        {
            if (sipxConfigSetAudioCodecByName(g_hInst, szCodec) == SIPX_RESULT_FAILURE)
            {
                printf("!! Setting audio codec to %s failed !!\n", szCodec);
            };
        }
        // Wait for a STUN response (should actually look for the STUN event status
        // (config event) ;
        SLEEP(1500) ;


        for (int i=0; i<iRepeatCount; i++)
        {
            ClearSipXEvents() ;

            printf("<-> Attempt %d of %d\n", i+1, iRepeatCount) ;

            // Place a call to designed URL
            if (placeCall(szSipUrl, szFromIdentity, szUsername, szPassword, szRealm))
            {
                bError = false ;

                // If the mic gain was expelicitly given
                if(inputGain >= 0.0)
                {
                    printf("Setting mic gain to: %f return code: %d\n", inputGain,
                        sipxCallSetMicGain(g_hCall, inputGain));
                }

                // If the speaker volume was explicitily set
                if(outputVolume >= 0)
                {
                    printf("Setting speaker volume to: %d return code: %d\n", outputVolume,
                        sipxAudioSetVolume(g_hInst, SPEAKER, outputVolume));
                }

                // Play tones if provided
                if (szPlayTones)
                {
                    if (!playTones(szPlayTones))
                    {
                        printf("%s: Failed to play tones: %s\n", argv[0], szPlayTones) ;
                    }
                    else
                    {
                        bError = true ;
                    }
                }

                // Play file if provided
                if (szFile)
                {
                    if (!playFile(szFile))
                    {
                        printf("%s: Failed to play file: %s\n", argv[0], szFile) ;
                    }
                    else
                    {
                        bError = true ;
                    }
                }

                // Play file from buffer if provided
                if (szFileBuffer)
                {
                    if (!playFileBuffer(szFileBuffer))
                    {
                        printf("%s: Failed to play file from buffer: %s\n", argv[0], szFileBuffer) ;
                    }
                    else
                    {
                        bError = true ;
                    }
                }


                // Leave the call up for specified time period (or wait for hangup)
                WaitForSipXEvent(CALLSTATE_DISCONNECTED, iDuration) ;

                // Shutdown / cleanup
                if (!shutdownCall())
                {
                    printf("%s: Failed to shutdown call\n", argv[0]) ;
                    bError = true ;
                }
            }
            else
            {
                printf("%s: Unable to complete call\n", argv[0]) ;
                shutdownCall() ;
                bError = true ;
            }            

            if (bError)
            {
                break ;
            }
        }        
        sipxEventListenerRemove(g_hInst, EventCallBack, NULL) ;

        sipxUnInitialize(g_hInst, true);
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
