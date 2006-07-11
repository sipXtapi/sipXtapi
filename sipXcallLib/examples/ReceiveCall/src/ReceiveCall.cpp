//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "os/OsDefs.h"

#include <assert.h>

#if defined(_WIN32)
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "os/OsDefs.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define SAMPLES_PER_FRAME   80          // Number of samples per frame time
#define LOOPBACK_LENGTH     200         // Frames for loopback delay (10ms per frame)

static short* g_loopback_samples[LOOPBACK_LENGTH] ; // loopback buffer
static short g_loopback_head = 0 ;      // index into loopback
static char* g_szPlayTones = NULL ;     // tones to play on answer
static char* g_szFile = NULL ;          // file to play on answer

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
    printf("   -l loopback audio (2 second delay)\n") ;
    printf("   -i line identity (e.g. sip:122@pingtel.com)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -x proxy (outbound proxy)\n");
    printf("   -S stun server\n") ;
    printf("   -v show sipXtapi version\n");
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char *argv[],
               int* pDuration,
               int* pSipPort,
               int* pRtpPort,
               char** pszPlayTones,
               char** pszFile,
               bool* bLoopback,
               char** pszIdentity,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszStunServer,
               char** pszProxy)
{
    bool bRC = true ;
    char szBuffer[64];

    assert(pDuration && pszPlayTones) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *bLoopback = false ;
    *pszIdentity = NULL ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszStunServer = NULL ;
    *pszProxy = NULL;

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
        else if (strcmp(argv[i], "-l") == 0)
        {
            *bLoopback = true ;
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
    bool bRC = false ;
    sipxCallPlayFile(hCall, g_szFile, true, true) ;

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
                sipxCallStartTone(hCall, (TONE_ID) toneId, true, false) ;
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
            sipxCallAccept(pCallInfo->hCall) ;
            break ;
        case CALLSTATE_ALERTING:
            clearLoopback() ;
            sipxCallAnswer(pCallInfo->hCall) ;
            break ;
        case CALLSTATE_CONNECTED:
            SLEEP(1000) ;   // BAD: Do not block the callback thread

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
                if (!playTones(g_szPlayTones, pCallInfo->hCall))
                {
                    printf("Failed to play tones: %s\n", g_szPlayTones) ;
                }
            }
            break ;
        case CALLSTATE_DISCONNECTED:
            sipxCallDestroy(pCallInfo->hCall) ;
            break ;
        case CALLSTATE_AUDIO_EVENT:
            if (pCallInfo->cause == CALLSTATE_AUDIO_START)
            {
                printf("* Negotiated codec: %s, payload type %d\n", pCallInfo->codecs.audioCodec.cName, pCallInfo->codecs.audioCodec.iPayloadType);
            }
            break;
        case CALLSTATE_DESTROYED:
            break ;
        }
    }
    return true;
}


SIPX_LINE lineInit(SIPX_INST hInst, char* szIdentity, char* szUsername, char* szPassword, char* szRealm)
{
    SIPX_LINE hLine = NULL ;

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


int main(int argc, char* argv[])
{
    bool bError = true ;
    int iDuration, iSipPort, iRtpPort ;
    bool bLoopback ;
    char* szIdentity ;
    char* szUsername ;
    char* szPassword ;
    char* szRealm ;
    char* szStunServer ;
    char* szProxy ;
    SIPX_INST hInst ;
    SIPX_LINE hLine ;


    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &g_szPlayTones,
        &g_szFile, &bLoopback, &szIdentity, &szUsername, &szPassword, &szRealm, &szStunServer, &szProxy) &&
        (iDuration > 0) && (portIsValid(iSipPort)) && (portIsValid(iRtpPort)))
    {
        if (bLoopback)
        {
            initLoopback() ;
        }

        // initialize sipx TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("ReceiveCall.log");
        if (sipxInitialize(&hInst, iSipPort, iSipPort, 5061, iRtpPort, 16, szIdentity) == SIPX_RESULT_SUCCESS)
        {            
            if (szProxy)
            {
                sipxConfigSetOutboundProxy(hInst, szProxy);
            }
            sipxConfigEnableRport(hInst, true) ;
            if (szStunServer)
            {
                sipxConfigEnableStun(hInst, szStunServer, 28) ;
            }
            sipxEventListenerAdd(hInst, EventCallBack, NULL) ;
            hLine = lineInit(hInst, szIdentity, szUsername, szPassword, szRealm) ;

            dumpLocalContacts(hInst) ;

            while (true)
            {
                SLEEP(1000) ;
            }
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

    return (int) bError ;
}

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */
