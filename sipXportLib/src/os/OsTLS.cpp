//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifdef SIP_TLS_NSS

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsTLS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "utl/UtlInt.h"

#include "prerror.h"
#include "pk11func.h"
#include "secitem.h"
#include "nspr.h"
#include "plgetopt.h"
#include "prio.h"
#include "prnetdb.h"
#include "nss.h"
#include "ssl.h"
#include "sslerr.h"
#include "secerr.h"
#include "sslproto.h"
#include "os/OsTLSConnectionSocket.h"

UtlHashMap* OsTLS::mpSinkMap = NULL;
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTLS::OsTLS() 
{
}


// Destructor
OsTLS::~OsTLS()
{
}

/* ============================ MANIPULATORS ============================== */
OsStatus OsTLS::Initialize(const char* szCertDir)
{
    // TODO - must implement locking
    OsStatus rc = OS_FAILED;
    static bInitialized = false;
    if (!bInitialized)
    {
        PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

        /* Initialize the NSS libraries. */
        {
            SECStatus secStatus = NSS_Init(szCertDir);
            if (secStatus == SECSuccess)
            {
                PK11_SetPasswordFunc(OsTLS::PasswordCallback);
                /* All cipher suites except RSA_NULL_MD5 are enabled by Domestic Policy. */
                NSS_SetExportPolicy();
                SSL_CipherPrefSetDefault(SSL_RSA_WITH_NULL_MD5, PR_TRUE);
                rc = OS_SUCCESS;
                bInitialized = true;
            }
        }
    }
    else 
    {
        rc = OS_SUCCESS;
    }
    return rc;
}

SECStatus OsTLS::GetClientAuthData(void *arg,
                                   PRFileDesc *socket,
                                   struct CERTDistNamesStr *caNames,
                                   struct CERTCertificateStr **pRetCert,
                                   struct SECKEYPrivateKeyStr **pRetKey) 
{

    CERTCertificate *  cert;
    SECKEYPrivateKey * privKey;
    char *             chosenNickName = (char *)arg;
    void *             proto_win      = NULL;
    SECStatus          secStatus      = SECFailure;

    proto_win = SSL_RevealPinArg(socket);

    if (chosenNickName)
    {
        cert = PK11_FindCertFromNickname(chosenNickName, proto_win);
        if (cert)
        {
            privKey = PK11_FindKeyByAnyCert(cert, proto_win);
            if (privKey) 
            {
                secStatus = SECSuccess;
            }
            else 
            {
                CERT_DestroyCertificate(cert);
            }
        }
    }
    else 
    { /* no nickname given, automatically find the right cert */
        CERTCertNicknames *names;
        int                i;

        names = CERT_GetCertNicknames(CERT_GetDefaultCertDB(), 
            SEC_CERT_NICKNAMES_USER, proto_win);

        if (names != NULL)
        {
            for(i = 0; i < names->numnicknames; i++ )
            {

                cert = PK11_FindCertFromNickname(names->nicknames[i], 
                    proto_win);
                if (!cert)
                {
                    continue;
                }

                /* Only check unexpired certs */
                if (CERT_CheckCertValidTimes(cert, PR_Now(), PR_FALSE)
                    != secCertTimeValid )
                {
                        CERT_DestroyCertificate(cert);
                        continue;
                }

                    secStatus = NSS_CmpCertChainWCANames(cert, caNames);
                    if (secStatus == SECSuccess)
                    {
                        privKey = PK11_FindKeyByAnyCert(cert, proto_win);
                        if (privKey)
                        {
                            break;
                        }
                        secStatus = SECFailure;
                        break;
                    }
            } /* for loop */
            CERT_FreeNicknames(names);
        }
    }
    if (secStatus == SECSuccess)
    {
        *pRetCert = cert;
        *pRetKey  = privKey;
    }
 
    return secStatus;
}

SECStatus OsTLS::AuthCertificate(void *arg, PRFileDesc *socket, 
                   PRBool checksig, PRBool isServer) 
{
 
    SECCertUsage        certUsage;
    CERTCertificate *   cert;
    void *              pinArg;
    char *              hostName;
    SECStatus           secStatus = SECFailure;
 
    if (!arg || !socket)
    {
    }

    /* Define how the cert is being used based upon the isServer flag. */

    certUsage = isServer ? certUsageSSLClient : certUsageSSLServer;

    cert = SSL_PeerCertificate(socket);

    pinArg = SSL_RevealPinArg(socket);

    /* SSL_RevealURL returns a hostName, not an URL. */
    hostName = SSL_RevealURL(socket);

    if (hostName && hostName[0])
    {
        bool bSuccess = false;
        ITlsSink* pSink = NULL;
        pSink = OsTLS::getTlsSink(socket);

        bSuccess = pSink->onServerCertificate(cert, hostName);
        if (bSuccess)
        {
            secStatus = SECSuccess;
        }

    } 
    else 
    {
        PR_SetError(SSL_ERROR_BAD_CERT_DOMAIN, 0);
        secStatus = SECFailure;
    }


    if (hostName)
    {
        PR_Free(hostName);
    }

    CERT_DestroyCertificate(cert);
    return secStatus;
 }

SECStatus OsTLS::BadCertHandler(void *arg, PRFileDesc *socket) 
{
    return SECSuccess;
    SECStatus   secStatus = SECFailure;
    PRErrorCode err = PORT_GetError();

    /* If any of the cases in the switch are met, then we will proceed   */
    /* with the processing of the request anyway. Otherwise, the default */     
    /* case will be reached and we will reject the request.              */
    switch (err)
    {
        case SEC_ERROR_INVALID_AVA:
        case SEC_ERROR_INVALID_TIME:
        case SEC_ERROR_BAD_SIGNATURE:
        case SEC_ERROR_EXPIRED_CERTIFICATE:
        case SEC_ERROR_UNKNOWN_ISSUER:
        case SEC_ERROR_UNTRUSTED_CERT:
        case SEC_ERROR_CERT_VALID:
        case SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE:
        case SEC_ERROR_CRL_EXPIRED:
        case SEC_ERROR_CRL_BAD_SIGNATURE:
        case SEC_ERROR_EXTENSION_VALUE_INVALID:
        case SEC_ERROR_CA_CERT_INVALID:
        case SEC_ERROR_CERT_USAGES_INVALID:
        case SEC_ERROR_UNKNOWN_CRITICAL_EXTENSION:
            secStatus = SECSuccess;
            break;
        default:
            secStatus = SECFailure;
            break;
    }

    if (arg)
    {
        *(PRErrorCode *)arg = err;
    }

    return secStatus;
 }

SECStatus OsTLS::HandshakeCallback(PRFileDesc *socket, void *arg) 
{
    OsTLSConnectionSocket* pSocket = (OsTLSConnectionSocket*)arg;
    if (pSocket)
    {
        pSocket->setHandshakeComplete();
        return SECSuccess;
    }
    return SECFailure;
}

char* OsTLS::PasswordCallback(PK11SlotInfo *slot, PRBool retry, void *arg) 
{
    if (1 == retry)
    {
        return NULL;
    }
    char* szPass = (char*)PR_Malloc(strlen((char*)arg) + 1);
    memset(szPass, 0, strlen((char*)arg)+1);
    strncpy(szPass, (char*)arg, strlen((char*)arg));
    return (char*)szPass;
}

void OsTLS::setTlsSink(PRFileDesc* key, ITlsSink* pSink)
{
    if (!mpSinkMap)
    {
        mpSinkMap = new UtlHashMap();
    }
    mpSinkMap->insertKeyAndValue(new UtlInt((int)key), new UtlInt((int) pSink));
}

ITlsSink* OsTLS::getTlsSink(PRFileDesc* key)
{
    ITlsSink* pSink = NULL;
    UtlInt* pSinkContainer = NULL;

    pSinkContainer = (UtlInt*)mpSinkMap->findValue(&UtlInt((int)key));

    pSink = (ITlsSink*) pSinkContainer->getValue();
    return pSink;
}

void OsTLS::removeTlsSink(PRFileDesc* key)
{
    mpSinkMap->destroy(&UtlInt((int)key));
}
/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif