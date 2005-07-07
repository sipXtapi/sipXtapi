//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// APPLICATION INCLUDES
#include "os/OsSSL.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* defaultPublicCertificateFile = SIPX_CONFDIR "/ssl/ssl.crt";
const char* defaultPrivateKeyFile        = SIPX_CONFDIR "/ssl/ssl.key";

bool OsSSL::sInitialized = false;

/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor

OsSSL::OsSSL(const char* publicCertificatePath,
             const char* privateKeyPath
             )
{
   if (!sInitialized)
   {
      
      // Initialize random number generator before using SSL

      // TODO: this is a bad way to do this, and it probably needs to be fixed.
      //
      //       We should be using better randomness if possible, but at the very
      //       least we should be saving the current rand state in a file so that
      //       is not reset each time.  I think that on a modern Linux, this has
      //       no effect because OpenSSL will use /dev/urandom internally anyway
      //
      // This needs to be examined.

      /* make a random number and set the top and bottom bits */
      int seed[32];
      for (unsigned int i = 0; i < sizeof(seed)/sizeof(int);i++)
      {
         seed[i] = rand();
      }

      RAND_seed(seed,sizeof(seed));
      SSLeay_add_ssl_algorithms();

      // It is suggested by the OpenSSL group that embedded systems
      // only enable loading of error strings when debugging.
      // Perhaps this should be conditional?
      SSL_load_error_strings();
   
      sInitialized = true;
   }
   
   mCTX = SSL_CTX_new(SSLv23_method());

   if (mCTX)
   {
      char* pubCert;
      char* privKey;

      if (publicCertificatePath)
      {
         pubCert = const_cast<char*>(publicCertificatePath);
         privKey = const_cast<char*>(privateKeyPath);
      }
      else
      {
         pubCert = const_cast<char*>(defaultPublicCertificateFile);
         privKey = const_cast<char*>(defaultPrivateKeyFile);
      }
      
      if (SSL_CTX_use_certificate_file(mCTX, pubCert, SSL_FILETYPE_PEM) > 0)
      {
         if (SSL_CTX_use_PrivateKey_file(mCTX, privKey, SSL_FILETYPE_PEM) > 0)
         {
            if (SSL_CTX_check_private_key(mCTX))
            {
               OsSysLog::add(FAC_KERNEL, PRI_INFO
                             ,"OsSSL::_ %p CTX %p loaded key pair:\n"
                             "   public  '%s'\n"
                             "   private '%s'"
                             ,this, mCTX, pubCert, privKey);
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_ERR
                             ,"OsSSL::_ Private key '%s' does not match the certificate public key '%s'\n"
                             ,privKey, pubCert);
            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR
                          ,"OsSSL::_ Private key '%s' could not be initialized.\n"
                          ,privKey
                          );
         }
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::_ Public key '%s' could not be initialized.\n"
                       ,pubCert
                       );
      }

      // disable server connection caching
      // TODO: Investigate turning this on...
      SSL_CTX_set_session_cache_mode(mCTX, SSL_SESS_CACHE_OFF);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::_ SSL_CTX_new failed");
   }
}


// Destructor

OsSSL::~OsSSL()
{
   // Since error queue data structures are allocated automatically for new threads,
   // they must be freed when threads are terminated in order to avoid memory leaks.
   ERR_remove_state(0);

   if (mCTX)
   {
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::~ SSL_CTX free %p", mCTX);
      SSL_CTX_free(mCTX);
      mCTX = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */
 
/* ============================ ACCESSORS ================================= */

/// Get an SSL server connection handle
SSL* OsSSL::getServerConnection()
{
   SSL* server = SSL_new(mCTX);
   if (server)
   {
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::getServerConnection returning %p", server);
      // SSL_set_accept_state(server);
      // SSL_set_options(server, SSL_OP_NO_SSLv2);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::getServerConnection SSL_new failed.");
   }

   return server;
}

/// Get an SSL client connection handle
SSL* OsSSL::getClientConnection()
{
   SSL* client = SSL_new(mCTX);
   if (client)
   {
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::getClientConnection returning %p", client);
      // SSL_set_connect_state(client);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::getClientSSL SSL_new failed.");
   }

   return client;
}

/// Release an SSL connection handle
void OsSSL::releaseConnection(SSL*& connection)
{
   if (connection)
   {
      SSL_free(connection);
      ERR_remove_state(0);
      connection = NULL;
   }
}

void OsSSL::logConnectParams(const OsSysLogFacility facility, ///< callers facility
                             const OsSysLogPriority priority, ///< log priority
                             const char* callerMsg,  ///< Identifies circumstances of connection
                             SSL*  connection  ///< SSL connection to be described
                             )
{
   if (connection)
   {
      char* subjectStr = NULL;
      char* issuerStr = NULL;

      // Extract the subject and issuer information about the peer
      //    (dynamic memory - freed below)
      X509* server_cert = SSL_get_peer_certificate(connection);
      if (server_cert)
      {
         subjectStr = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
         issuerStr = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
      }
   
      // Get the name of the encryption applied to the connection
      const char* cipher = SSL_get_cipher(connection);

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s SSL Connection:\n"
                    "   subject: %s\n"
                    "   issuer: %s\n"
                    "   cipher: %s",
                    callerMsg,
                    subjectStr ? subjectStr : "(none)",
                    issuerStr  ? issuerStr  : "(none)",
                    cipher     ? cipher     : "(none)"
                    );

      // Release the subject and issue descriptions
      if (subjectStr)
      {
         OPENSSL_free(subjectStr);
      }
      if (issuerStr)
      {
         OPENSSL_free(issuerStr);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsSSL::logConnectParams called by %s with NULL connection",
                    callerMsg
                    );
   }
}


void OsSSL::logError(const OsSysLogFacility facility,
                     const OsSysLogPriority priority,
                     const char* callerMsg,
                     int errCode)
{
   char sslErrorString[256];
   ERR_error_string_n(errCode, sslErrorString, sizeof(sslErrorString));
   OsSysLog::add(facility, priority,
                 "%s:\n   SSL error: %s",
                 callerMsg, sslErrorString
                 );
}

/********************************************************************************/


OsSSL* OsSharedSSL::get()
{
   OsSSL* shared = (  sharedSSL
                    ? sharedSSL
                    : sharedSSL = new OsSSL()
                    );

#  if 1 // TBD
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSharedSSL::get returning %p", shared);
#  endif

   return shared;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsSSL* OsSharedSSL::sharedSSL = NULL;

/* //////////////////////////// PROTECTED ///////////////////////////////// */



/* ============================ FUNCTIONS ================================= */
