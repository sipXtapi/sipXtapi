//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTLSServerSocket_h_
#define _OsTLSServerSocket_h_

#ifdef SIP_TLS
#ifdef SIP_TLS_NSS

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <os/OsTLSConnectionSocket.h>
#include <os/OsFS.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
enum TlsInitCodes
{
    TLS_INIT_SUCCESS,
    TLS_INIT_DATABASE_FAILURE,
    TLS_INIT_BAD_PASSWORD,
    TLS_INIT_TCP_IMPORT_FAILURE,
    TLS_INIT_NSS_FAILURE,
};

class OsTLSServerSocket : public OsServerSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor to set up TCP socket server
   OsTLSServerSocket(int connectionQueueSize, /**< The maximum number of outstanding connection
                                               *   requests which are allowed before subsequent
                                               *   requests are turned away.*/ 
                     int serverPort=PORT_DEFAULT, /**< The port on which the server will listen to
                                                  *   accept connection requests.
                                                  *   PORT_DEFAULT means let OS pick port. */
                     UtlString certNickname = "",
                     UtlString certPassword = "",
                     UtlString dbLocation = "",
                     const UtlString bindAddress = ""                                               
                     );
   /**
    * Sets the socket connection queue and starts listening on the
    * port for requests to connect.
    */

   /// Assignment operator
   OsTLSServerSocket& operator=(const OsTLSServerSocket& rhs);

  virtual
   ~OsTLSServerSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual OsConnectionSocket* accept();
   //:Blocking accept of next connection
   // Blocks and waits for the next TCP connection request.
   //!returns: Returns a socket connected to the client requesting the
   //!returns: connection.  If an error occurs returns NULL.


   virtual void close();
   //: Close down the server

/* ============================ ACCESSORS ================================= */

   virtual int getLocalHostPort() const;
   //:Return the local port number
   // Returns the port to which this socket is bound on this host.

/* ============================ INQUIRY =================================== */

   virtual int getIpProtocol() const;
   //: Returns the protocol type of this socket

   virtual UtlBoolean isOk() const;
   //: Server socket is in ready to accept incoming conection requests.

   int isConnectionReady();
   //: Poll to see if connections are waiting
   //!returns: 1 if one or call to accept() will not block <br>
   //!returns: 0 if no connections are ready (i.e. accept() will block).

   TlsInitCodes getTlsInitCode() { return mTlsInitCode; }
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   
   static int pemPasswdCallbackFunc(char *buf, int size, int rwflag, void *userdata);
    //:Callback used to set the private key password for decrypting the key
    //:buf is the buffer to fill with a password
    //:size is the maximum size of the buffer 

   OsTLSServerSocket(const OsTLSServerSocket& rOsTLSServerSocket);
     //:Disable copy constructor

   OsTLSServerSocket();
     //:Disable default constructor

   UtlString mCertNickname;
   UtlString mCertPassword;
   UtlString mDbLocation;
   
   PRFileDesc* mpMozillaSSLSocket;
   SECKEYPrivateKey *  mpPrivKey;
   CERTCertificate *   mpCert;
   TlsInitCodes mTlsInitCode;
   
};

/* ============================ INLINE METHODS ============================ */

#endif // SIP_TLS
#endif // SIP_TLS_NSS

#endif  // _OsTLSServerSocket_h_

