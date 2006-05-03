//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

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

#define MAX_RECORD_EVENTS       16

SIPX_INST g_hInst = NULL ;      // Handle to the sipXtapi instanance
SIPX_LINE g_hLine = 0 ;         // Line Instance (id, auth, etc)
SIPX_SUB  g_hSub = 0 ;          // Handle to sub scriptions


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
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -d durationInSeconds (default=10 seconds)\n") ;
    printf("   -u username (for authentication)\n") ;
    printf("   -a password  (for authentication)\n") ;
    printf("   -m realm  (for authentication)\n") ;
    printf("   -i from identity\n") ;
    printf("   -S stun server\n") ;
    printf("   -x proxy (outbound proxy)\n");
    printf("\n") ;
}


// Parse arguments
bool parseArgs(int argc,
               char*  argv[],
               int*   pDuration,
               int*   pSipPort,
               char** pszUrl,
               char** pszUsername,
               char** pszPassword,
               char** pszRealm,
               char** pszFromIdentity,
               char** pszStunServer,
               char** pszProxy)
{
    bool bRC = false ;

    *pDuration = 10 ;
    *pSipPort = 5060 ;
    *pszUrl = NULL ;
    *pszUsername = NULL ;
    *pszPassword = NULL ;
    *pszRealm = NULL ;
    *pszFromIdentity = NULL ;
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

    if (category == EVENT_CATEGORY_NOTIFY)
    {
        SIPX_NOTIFY_INFO* pNI = (SIPX_NOTIFY_INFO*) pInfo ;
        printf(" == %s / %s / %d bytes == \n", 
                (pNI->szNotiferUserAgent != NULL) ? pNI->szNotiferUserAgent : "",
                (pNI->szContentType != NULL) ? pNI->szContentType : "",
                pNI->nContentLength) ;
        printf("%s\n\n", (char*) (pNI->pContent != NULL) ? pNI->pContent : "") ;
    }

    return true ;
}

int findBestContact()
{
    int iContactId = 1;

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

            const char* szTransportType = "UNKNOWN" ;
            switch (contacts[i].eTransportType)
            {
                case TRANSPORT_UDP:
                    szTransportType = "UDP" ;
                    break ;
                case TRANSPORT_TCP:
                    szTransportType = "TCP" ;
                    break ;
                case TRANSPORT_TLS:
                    szTransportType = "TLS" ;
                    break ;
                default:
                    assert(false) ;
            }

            printf("<-> Type %s, Interface: %s/%s, Ip %s, Port %d\n",
                    szType, contacts[i].cInterface, szTransportType, 
                    contacts[i].cIpAddress, contacts[i].iPort) ;

            if (contacts[i].eContactType == CONTACT_NAT_MAPPED) 
            {
                iContactId = contacts[i].id ;
            }
        }
    }
    else
    {
        printf("<-> Unable to query local contact addresses\n") ;
    }

    return iContactId ;
}


// Place a call to szSipUrl as szFromIdentity
bool checkVMail(char* szSipUrl, char* szFromIdentity, char* szUsername, char* szPassword, char *szRealm)
{
    bool bRC = false ;

    if ((szFromIdentity == NULL) || strlen(szFromIdentity) == 0)
    {
        szFromIdentity = "\"PlaceCall Demo\" <sip:placecalldemo@localhost>" ;
    }

    sipxLineAdd(g_hInst, szFromIdentity, &g_hLine) ;
    if (szUsername && szPassword && szRealm)
    {
        sipxLineAddCredential(g_hLine, szUsername, szPassword, szRealm) ;
    }

    int contactID = findBestContact() ;

    sipxConfigSubscribe(g_hInst,
                        g_hLine, 
                        szSipUrl,   
                        "message-summary", 
                        "application/simple-message-summary",
                        1, 
                        &g_hSub); 

    return bRC ;
}


int main(int argc, char* argv[])
{
    bool bError = false ;
    int iDuration ;
    int iSipPort ;
    char* szSipUrl;
    char* szUsername;
    char* szPassword;
    char* szRealm;
    char* szFromIdentity;
    char* szStunServer;
    char* szProxy;

    // Parse Arguments
    if (parseArgs(argc, argv, &iDuration, &iSipPort, &szSipUrl, &szUsername, &szPassword, 
            &szRealm, &szFromIdentity, &szStunServer, &szProxy)            
            && (portIsValid(iSipPort)))
    {
        // initialize sipx TAPI-like API
        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG) ;
        sipxConfigSetLogFile("CheckVMail.log");
        sipxInitialize(&g_hInst, iSipPort, iSipPort, PORT_NONE, 8000);
        sipxConfigEnableRport(g_hInst, true) ;
        sipxEventListenerAdd(g_hInst, EventCallBack, NULL) ;

        if (szProxy)
        {
            sipxConfigSetOutboundProxy(g_hInst, szProxy);
        }

        if (szStunServer)
        {
            sipxConfigEnableStun(g_hInst, szStunServer, DEFAULT_STUN_PORT, 28) ;
        }
       
        checkVMail(szSipUrl, szFromIdentity, szUsername, szPassword, szRealm) ;

        Sleep(iDuration * 1000) ;
       
        sipxConfigUnsubscribe(g_hSub) ;
        Sleep(2000) ;
        sipxLineRemove(g_hLine) ;
        sipxEventListenerRemove(g_hInst, EventCallBack, NULL) ;
        sipxUnInitialize(g_hInst);        
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
