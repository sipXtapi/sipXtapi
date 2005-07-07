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

#include <windows.h>

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
	printf("\nUsage:\n") ;
	printf("   %s <options>\n", szExecutable) ;
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
    printf("   -S stun server\n") ;
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
               char** pszStunServer)
{
	bool bRC = true ;	

	assert(pDuration && pszPlayTones) ;
	*pDuration = 30 ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
	*pszPlayTones = NULL ;
    *pszFile = NULL ;
    *bLoopback = FALSE ;
    *pszIdentity = NULL ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszStunServer = NULL ;

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
				break ;	// Error
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
				break ;	// Error
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
				break ;	// Error
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
				break ;	// Error
			}
		}
        else if (strcmp(argv[i], "-l") == 0)
		{
            *bLoopback = TRUE ;
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
				break ;	// Error
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
				break ;	// Error
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
				break ;	// Error
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
				break ;	// Error
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
				break ;	// Error
			}
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
                (toneId == '#') || (toneId == '*') || toneId == ',')
        {
            if (toneId == ',')
            {
                printf("<-> Playtone: Sleeping for 2 seconds\n") ;
                Sleep(2000) ;
            }
            else
            {
                printf("<-> Playtone: %c\n", toneId) ;
                Sleep(250) ;
                sipxCallStartTone(hCall, (TONE_ID) toneId, true, false) ;
                Sleep(500) ;
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


void EventCallBack( SIPX_CALL hCall, 
                    SIPX_LINE hLine, 
					SIPX_CALLSTATE_MAJOR eMajor, 
					SIPX_CALLSTATE_MINOR eMinor, 
                    void* pUserData) 
{
    switch (eMajor)
    {
        case OFFERING:
            printf("LineId: %d\n", hLine) ;
            sipxCallAccept(hCall) ;
            break ;
        case ALERTING:
            clearLoopback() ;
            sipxCallAnswer(hCall) ;
            break ;
        case CONNECTED:
            Sleep(1000) ;   // BAD: Do not block the callback thread

            // Play file if provided
            if (g_szFile)
            {
                if (!playFile(g_szFile, hCall))
                {
                    printf("Failed to play file: %s\n", g_szFile) ;
                }
            }

            // Play tones if provided
            if (g_szPlayTones)
            {
                if (!playTones(g_szPlayTones, hCall))
                {
                    printf("Failed to play tones: %s\n", g_szPlayTones) ;
                }
            }
            break ;
        case DISCONNECTED:
            sipxCallDestroy(hCall) ;
            break ;
    }
}


SIPX_LINE lineInit(SIPX_INST hInst, char* szIdentity, char* szUsername, char* szPassword, char* szRealm) 
{
    SIPX_LINE hLine = NULL ;

    if (szIdentity && strlen(szIdentity))
    {
        sipxLineAdd(hInst, szIdentity, TRUE, &hLine) ;

        if (    szUsername && strlen(szUsername) &&
                szPassword && strlen(szPassword) &&
                szRealm && strlen(szRealm))
        {
            sipxLineAddCredential(hLine, szUsername, szPassword, szRealm) ;
        }
    }

    return hLine ;
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
    SIPX_INST hInst ;
    SIPX_LINE hLine ;


    // Parse Arguments
	if (parseArgs(argc, argv, &iDuration, &iSipPort, &iRtpPort, &g_szPlayTones, 
            &g_szFile, &bLoopback, &szIdentity, &szUsername, &szPassword, &szRealm, &szStunServer) &&
			(iDuration > 0) && (iSipPort > 0) && (iRtpPort > 0))
	{
        if (bLoopback)
        {
            initLoopback() ;
        }

        // initialize sipx TAPI-like API
        sipxConfigEnableLog("ReceiveCall.log", LOG_LEVEL_INFO) ;
        if (sipxInitialize(&hInst, iSipPort, iSipPort, iRtpPort) == SIPX_RESULT_SUCCESS)
        {
            sipxConfigEnableRport(hInst, TRUE) ;
            if (szStunServer)
            {
                sipxConfigEnableStun(hInst, szStunServer, 28) ;
            }
            sipxListenerAdd(hInst, EventCallBack, NULL) ;
            hLine = lineInit(hInst, szIdentity, szUsername, szPassword, szRealm) ;
            
            while (true) 
            {
                Sleep(1000) ;
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
