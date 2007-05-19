//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsTLS_h_
#define _OsTLS_h__



// SYSTEM INCLUDES

// APPLICATION INCLUDES                      
#include "os/OsBSem.h"
#include "os/OsSysLog.h"
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"

#include <nspr.h>
#include <seccomon.h>
#include <secmod.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                                       
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_CERT_DIR_LENGTH 256
// STRUCTS
// ENUMERATIONS
typedef enum SECURITY_CAUSE
{
    E_SECURITY_CAUSE_UNKNOWN = 0,                      /**< An UNKNOWN cause code is generated when the state
                                                          for the security operation 
                                                          is no longer known.  This is generally an error 
                                                          condition; see the info structure for details. */
    E_SECURITY_CAUSE_NORMAL,                           /**< Event was fired as part of the normal encryption / decryption process. */
    E_SECURITY_CAUSE_ENCRYPT_SUCCESS,                  /**< An S/MIME encryption succeeded. */
    E_SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT,         /**< An S/MIME encryption failed because the
                                                          security library could not start. */
    E_SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY,   /**< An S/MIME encryption failed because of a bad certificate / public key. */
    E_SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME encryption failed because of an invalid parameter. */
    E_SECURITY_CAUSE_DECRYPT_SUCCESS,                  /**< An S/MIME decryption succeeded. */ 
    E_SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT,          /**< An S/MIME decryption failed due to a failure to initialize the certificate database. */
    E_SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD,  /**< An S/MIME decryption failed due to an invalid certificate database password. */
    E_SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME decryption failed due to an invalid parameter. */
    E_SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE,            /**< An S/MIME decryption operation aborted due to a bad signature. */
    E_SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE,        /**< An S/MIME decryption operation aborted due to a missing signature. */
    E_SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED,       /**< An S/MIME decryption operation aborted because the signature was rejected. */
    E_SECURITY_CAUSE_TLS_SERVER_CERTIFICATE,
    E_SECURITY_CAUSE_TLS_BAD_PASSWORD,
    E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE,
    E_SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE,
    E_SECURITY_CAUSE_TLS_CONNECTION_FAILURE,
    E_SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE,
    E_SECURITY_CAUSE_SIGNATURE_NOTIFY,                /**< The SIGNATURE_NOTIFY event is fired when the user-agent
                                                         receives a SIP message with signed SMIME as its content.
                                                         The signer's certificate will be located in the info structure
                                                         associated with this event.  The application can choose to accept
                                                         the signature, by returning 'true' in response to this message
                                                         or can choose to reject the signature
                                                         by returning 'false' in response to this message. */
    E_SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED         /** < The application has rejected the server's TLS certificate. */
} SECURITY_CAUSE;

// TYPEDEFS
// FORWARD DECLARATIONS


class ITlsSink
{
public:
    virtual bool onServerCertificate(void* pCert, char* serverHostName) = 0;
    virtual bool onTlsEvent(int cause) = 0;
};

#ifdef SIP_TLS_NSS
// Singleton class for invoking NSS functions.
class OsTLS
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
private:
    OsTLS();
public:
    virtual ~OsTLS();


    static OsStatus Initialize(const char* szCertDir);
    static SECStatus GetClientAuthData(void *arg,
                                   PRFileDesc *socket,
                                   struct CERTDistNamesStr *caNames,
                                   struct CERTCertificateStr **pRetCert,
                                   struct SECKEYPrivateKeyStr **pRetKey);

    static SECStatus AuthCertificate(void *arg,
                                     PRFileDesc *socket,
                                     PRBool checksig,
                                     PRBool isServer) ;

    static SECStatus BadCertHandler(void *arg, PRFileDesc *socket);
    static SECStatus HandshakeCallback(PRFileDesc *socket, void *arg); 
    static char* PasswordCallback(PK11SlotInfo *slot, PRBool retry, void *arg);
    
    static void setTlsSink(PRFileDesc* key, ITlsSink* pSink);
    static ITlsSink* getTlsSink(PRFileDesc* key);
    static void removeTlsSink(PRFileDesc* key);
private:

    static UtlHashMap* mpSinkMap;
};




/* ============================ INLINE METHODS ============================ */

#endif

#endif  // _OsTLS_h_
