//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsSSL_h_
#define _OsSSL_h__

// SYSTEM INCLUDES

// APPLICATION INCLUDES                      
#include "os/OsBSem.h"
#include "os/OsSysLog.h"
#include "utl/UtlString.h"
#include "openssl/ssl.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                                       
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Wrapper for the OpenSSL SSL_CTX context structure.
/// This class is responsible for all global policy initialization and
/// enforcement.
class OsSSL
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   /// Construct an SSL Context from which connections are created.
   OsSSL(const char* authorityPath = NULL,   /**< Path to a directory containing trusted
                                              *   certificates files;
                                              * If NULL, compiled-in default is used */
         const char* publicCertificatePath = NULL, /**< Path to certificate file;
                                                    * If NULL, compiled-in default is used */
         const char* privateKeyPath = NULL  /**< Path to private key file;
                                             * If NULL, compiled-in default is used.
                                             * @note: If publicCertificatePath is NULL, this
                                             *        must also be NULL.
                                             */
         );

   ~OsSSL();

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */
 
/* ============================ ACCESSORS ================================= */

   /// Get an SSL server connection handle
   SSL* getServerConnection();

   /// Get an SSL client connection handle
   SSL* getClientConnection();

   /// Release an SSL session handle
   void releaseConnection(SSL*& connection);

   /// Log SSL connection information
   static void logConnectParams(const OsSysLogFacility facility, ///< callers facility
                                const OsSysLogPriority priority, ///< log priority
                                const char* callerMsg,  ///< Identifies circumstances of connection
                                SSL*  connection  ///< SSL connection to be described
                                );
   

   /// Log an error resulting from an SSL call, with the SSL error text expanded
   static void logError(const OsSysLogFacility facility, ///< callers facility
                        const OsSysLogPriority priority, ///< how bad was it?
                        const char* callerMsg,  ///< Identifies caller and what failed
                        int errCode             ///< error returned from ssl routine
                        );
   
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   static bool sInitialized;
   
   SSL_CTX* mCTX;

   /// Certificate chain validation hook called by openssl
   static int verifyCallback(int valid,            ///< validity so far from openssl
                             X509_STORE_CTX* store ///< certificate information db
                             );
   /**<
    * @returns validity as determined by local policy
    * @note See 'man SSL_CTX_set_verify'
    */

   /// Disable copy constructor
   OsSSL(const OsSSL& rOsSSL);

   /// Disable assignment operator
   OsSSL& operator=(const OsSSL& rhs);
};

/// A singleton wrapper for OsSSL
class OsSharedSSL
{
  public:
   
   static OsSSL* get();
   
  private:

   static OsBSem* spSslLock;
   static OsSSL*  spSharedSSL;
   
   /// singleton constructor
   OsSharedSSL();

   ~OsSharedSSL();
   
   /// Disable copy constructor
   OsSharedSSL(const OsSharedSSL& r);

   /// Disable assignment operator
   OsSharedSSL& operator=(const OsSharedSSL& rhs);
};



/* ============================ INLINE METHODS ============================ */

#endif  // _OsSSL_h_
