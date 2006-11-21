//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsNatServerSocket_h_
#define _OsNatServerSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>

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

class OsNatServerSocket : public OsServerSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsNatServerSocket(int connectionQueueSize,
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
   
   OsNatServerSocket& operator=(const OsServerSocket& rhs);
     //:Assignment operator

  virtual
   ~OsNatServerSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   virtual OsConnectionSocket* createConnectionSocket(UtlString localIp, int descriptor);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsNatServerSocket(const OsServerSocket& rOsServerSocket);
     //:Disable copy constructor

   OsNatServerSocket();
     //:Disable default constructor


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsNatServerSocket_h_