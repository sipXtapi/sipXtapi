//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsServerSocket_h_
#define _OsServerSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Implements TCP server for accepting TCP connections
// This class provides the implementation of the UDP datagram-based 
// socket class which may be instantiated. 

class OsServerSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsServerSocket(int connectionQueueSize,
                  int serverPort=PORT_DEFAULT,
                  const char* szBindAddr = NULL,
                  const bool bPerformBind = true);

   //:Constructor to set up TCP socket server
   // Sets the socket connection queue and starts listening on the
   // port for requests to connect.
   // 
   //!param: connectionQueueSize - The maximum number of outstanding
   // connection requests which are allowed before subsequent requests
   // are turned away.
   //!param: serverPort - The port on which the server will listen to
   // accept connection requests.  PORT_DEFAULT means let OS pick port.
   
   OsServerSocket& operator=(const OsServerSocket& rhs);
     //:Assignment operator

  virtual
   ~OsServerSocket();
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
   virtual UtlBoolean isOk() const;
   //: Server socket is in ready to accept incoming conection requests.

   int isConnectionReady();
   //: Poll to see if connections are waiting
   //!returns: 1 if one or call to accept() will not block <br>
   //!returns: 0 if no connections are ready (i.e. accept() will block).

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int socketDescriptor;
   int localHostPort;
   UtlString mLocalIp;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsServerSocket(const OsServerSocket& rOsServerSocket);
     //:Disable copy constructor

   OsServerSocket();
     //:Disable default constructor


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsServerSocket_h_

