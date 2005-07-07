// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define MAX_RECORD_EVENTS       16

SIPX_INST g_hInst = NULL ;      // Handle to the sipXtapi instanance
SIPX_LINE g_hLine = NULL ;      // Line Instance (id, auth, etc)
SIPX_CALL g_hCall = NULL ;      // Handle to a call

SIPX_CALLSTATE_MAJOR    g_eRecordEvents[MAX_RECORD_EVENTS] ;    // List of last N events
int                     g_iNextEvent ;      // Index for g_eRecordEvents ringer buffer


// Print usage message
void usage(const char* szExecutable)
{
    printf("\nUsage:\n") ;
    printf("   %s <options> [URL]\n", szExecutable) ;
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -d durationInSeconds (default=30 seconds)\n") ;
    printf("   -t play tones (default = none)\n") ;
    printf("   -f play file (default = none)\n") ;
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -r RTP port start (default = 9000)\n") ;    
    printf("   -R use rport as part of via (disabled by default)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -i from identity\n") ;
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc, 
               char *argv[], 
               int* pDuration, 
               int* pSipPort, 
               int* pRtpPort, 
               char** pszPlayTones, 
               char **pszFile, 
               char** pszUrl, 
               bool* bUseRport,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszFromIdentity) 
{
    bool bRC = false ;  

    assert(pDuration && pszPlayTones && pszUrl) ;
    *pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
    *pszPlayTones = NULL ;
    *pszFile = NULL ;
    *pszUrl = NULL ;
    *bUseRport = FALSE ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszFromIdentity = NULL ;

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
        else if (strcmp(argv[i], "-R") == 0)
        {
            *bUseRport = TRUE ;
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


// Event callback -- records last MAX_RECORD_EVENTS events
void EventCallbackProc( SIPX_CALL hCall,
						SIPX_LINE hLine,
                        SIPX_CALLSTATE_MAJOR eMajor,
						SIPX_CALLSTATE_MINOR eMinor,  
						void* pUser) 
{
    char* szEventDesc = sipxEventToString(eMajor, eMinor) ;
    printf("<-> Received Event: %s\n", szEventDesc) ;
    free(szEventDesc) ;
    
    g_eRecordEvents[g_iNextEvent] = eMajor ;
    g_iNextEvent = (g_iNextEvent + 1) % MAX_RECORD_EVENTS ;    
}

// Wait for the designated event for at worst ~iTimeoutInSecs seconds
bool WaitForSipXEvent(SIPX_CALLSTATE_MAJOR eMajor, int iTimeoutInSecs) 
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
            if (g_eRecordEvents[i] == eMajor)
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
        }
    }

    return bFound ;
}

// Clear the event log
void ClearSipXEvents()
{
    for (int i=0;i<MAX_RECORD_EVENTS; i++)
    {
        g_eRecordEvents[i] = UNKNOWN ;
    }
    g_iNextEvent = 0 ;
}


// Place a call to szSipUrl as szFromIdentity
bool placeCall(char* szSipUrl, char* szFromIdentity, char* szUsername, char* szPassword, char *szRealm)
{
    bool bRC = false ;

    if ((szFromIdentity == NULL) || strlen(szFromIdentity) == 0)
    {
        szFromIdentity = "\"PlaceCall Demo\" sip:placecalldemo@localhost" ;
    }

    printf("<-> Placing call to \"%s\" as \"%s\"\n", szSipUrl, szFromIdentity) ;    
    printf("<-> Username: %s, passwd: %s, realm: %s (all required for auth)\n", szUsername, szPassword, szRealm) ;

    sipxLineAdd(g_hInst, szFromIdentity, false, &g_hLine) ;
    if (szUsername && szPassword && szRealm)
    {
        sipxLineAddCredential(g_hLine, szUsername, szPassword, szRealm) ;
    }
    sipxCallCreate(g_hInst, g_hLine, &g_hCall) ;    
    sipxCallConnect(g_hCall, szSipUrl) ;
    bRC = WaitForSipXEvent(CONNECTED, 10) ;
   
    return bRC ;
}


// Drop call, clean up resources
bool shutdownCall()
{
    printf("<-> Shutting down Call\n") ;

    ClearSipXEvents() ;
    sipxCallDestroy(g_hCall) ;    
    sipxLineRemove(g_hLine) ;
    
    WaitForSipXEvent(DESTROYED, 2) ;

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
                (toneId == '#') || (toneId == '*') || toneId == ',')
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
                sipxCallStartTone(g_hCall, (TONE_ID) toneId, true, true) ;
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
    bool bRC = false ;
    sipxCallPlayFile(g_hCall, szFile, true, true) ;

    return true ;
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


int main(int argc, char* argv[])
{   
    bool bError = true ;
    int iDuration, iSipPort, iRtpPort ;
    char* szPlayTones;
    char* szSipUrl;
    char* szFile;
    char* szUsername;
    char* szPassword;
    char* szRealm;
    char* szFromIdentity;
    bool bUseRport ;

    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &szPlayTones, &szFile, &szSipUrl, 
            &bUseRport, &szUsername, &szPassword, &szRealm, &szFromIdentity) &&
            (iDuration > 0) && (iSipPort > 0) && (iRtpPort > 0))
    {       
        // initialize sipx TAPI-like API
        sipxInitialize(&g_hInst, iSipPort, iSipPort, iRtpPort);
        sipxConfigEnableRport(g_hInst, bUseRport) ;
        dumpInputOutputDevices() ;
        sipxListenerAdd(g_hInst, EventCallbackProc, NULL) ;

        // Place a call to designed URL
        if (placeCall(szSipUrl, szFromIdentity, szUsername, szPassword, szRealm))
        {
            bError = false ;

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
                free(szPlayTones) ;
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
                free(szFile) ;
            }


            // Leave the call up for specified time period (or wait for hangup)
            WaitForSipXEvent(DISCONNECTED, iDuration) ;

            // Shutdown / cleanup
            if (!shutdownCall())
            {
                printf("%s: Failed to shutdown call\n", argv[0]) ;
            }
            else
            {
                bError = true ;
            }
            free (szSipUrl) ;
        }
        else
        {
            printf("%s: Unable to complete call\n", argv[0]) ;
            shutdownCall() ;
        }

        sipxListenerRemove(g_hInst, EventCallbackProc, NULL) ;
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
