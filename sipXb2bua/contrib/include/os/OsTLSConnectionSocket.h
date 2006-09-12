//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTLSConnectionSocket_h_
#define _OsTLSConnectionSocket_h_

#ifdef SIP_TLS
#ifdef SIP_TLS_NSS

// SYSTEM INCLUDES
#include <prio.h>
#include <nspr.h>
#include <seccomon.h>
#include <secmod.h>
#include <ssl.h>

// APPLICATION INCLUDES                      
#include <os/OsConnectionSocket.h>

#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS                                       
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Implements TLS version of OsSocket
class OsTLSConnectionSocket : public OsConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsTLSConnectionSocket(int         remoteHostPort,
                         const char* remoteHostName,
                         const UtlString   certNickname,
                         const UtlString   password,
                         const UtlString   dbLocation,
                         long        timeoutInSecs,
                         const char* localIp
                         );

   OsTLSConnectionSocket(int socketDescriptor,
                         const UtlString certNickname,
                         const UtlString password,
                         const UtlString dbLocation,
                         long timeoutInSecs = 0); 

  virtual
   ~OsTLSConnectionSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
 
   virtual UtlBoolean reconnect();
   //: Sets up the connection again, assuming the connection failed

   virtual int write(const char* buffer, int bufferLength);
   //:Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //!param: buffer - The bytes to be written to the socket.
   //!param: bufferLength - The number of bytes contained in buffer.
   //!returns: The number of bytes actually written to the socket.
   //!returns: <br>Note: This does not necessarily mean that the bytes were 
   //!returns: actually received on the other end.

   virtual int write(const char* buffer, int bufferLength, long waitMilliseconds);
   //:Non-blocking or limited blocking write to socket
   // Same as blocking version except that this write will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero, in which case it does not block.

   virtual int read(char* buffer, int bufferLength);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of 
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength, 
                    UtlString* ipAddress, int* port);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of 
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!param: ipAddress - The address of the socket that sent the bytes read.
   //!param: port - The port of the socket that sent the bytes read.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength, long waitMilliseconds);
   //: Non-blocking or limited blocking read from socket
   // Same as blocking version except that this read will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero in which case it does not block.

   virtual UtlBoolean isReadyToReadEx(long waitMilliseconds, UtlBoolean &rSocketError) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is read to read.
   // Returns FALSE if wait expires or socket error.
   // rSocketError returns TRUE is socket error occurred.

   virtual UtlBoolean isReadyToRead(long waitMilliseconds = 0) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is read to read.
   // Returns FALSE if wait expires or socket error.

   virtual UtlBoolean isReadyToWrite(long timeoutMilliSec) const;

   void setHandshakeComplete() { mbHandshakeComplete = true; }
   bool waitForHandshake(long milliseconds) const;

/* ============================ ACCESSORS ================================= */

   virtual void close();
   //: Closes the TLS socket

/* ============================ INQUIRY =================================== */

   virtual int getIpProtocol() const;
   //: Returns the protocol type of this socket

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   virtual void NSSInitSocket(int socket, long timeoutInSecs, const char* szPassword) = 0;

   UtlBoolean mbExternalTLSSocket;

   OsTLSConnectionSocket(const OsTLSConnectionSocket& rOsTLSConnectionSocket);
     //:Disable copy constructor

   OsTLSConnectionSocket();
     //:Disable default constructor

   OsTLSConnectionSocket& operator=(const OsTLSConnectionSocket& rhs);
     //:Assignment operator

   PRFileDesc* mpPRfd;
   UtlString mCertNickname;
   UtlString mCertPassword;
   UtlString mDbLocation;
   SECKEYPrivateKey *  mpPrivKey;
   CERTCertificate *   mpCert;
   mutable OsMutex mSocketGuard;
   bool mbHandshakeComplete;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

};

/* ============================ INLINE METHODS ============================ */

#endif // SIP_TLS
#endif // SIP_TLS_NSS

#endif  // _OsTLSConnectionSocket_h_

