//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsConnectionSocket_h_
#define _OsConnectionSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Implements TCP version of OsSocket
// This class provides the implementation of the TCP datagram 
// based socket class which may be instantiated.
class OsConnectionSocket : public OsSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsConnectionSocket(int remoteHostPort,
                      const char* remoteHostName,
                      UtlBoolean makeBlocking = TRUE,
                      const char* localIp = NULL,
                      const bool bConnect = true);

   OsConnectionSocket(int connectedSocketDescriptor);

   OsConnectionSocket(const char* localIp,
                      int connectedSocketDescriptor);

  virtual
   ~OsConnectionSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
 
   virtual UtlBoolean reconnect();
   //: Sets up the connection again, assuming the connection failed

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

/* ============================ ACCESSORS ================================= */
   virtual int getIpProtocol() const;
   //: Returns the protocol type of this socket


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsConnectionSocket(const OsConnectionSocket& rOsConnectionSocket);
     //:Disable copy constructor

   OsConnectionSocket();
     //:Disable default constructor

   OsConnectionSocket& operator=(const OsConnectionSocket& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsConnectionSocket_h_

