//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsMulticastSocket_h_
#define _OsMulticastSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDatagramSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Implements multicast version of OsDatagramSocket
// This class provides the implementation of the multicast UDP datagram
class OsMulticastSocket : public OsDatagramSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsMulticastSocket(int multicastPort = PORT_DEFAULT,
                     const char* multicastHostName = NULL,
                     int localHostPort = PORT_DEFAULT,
                     const char* localHostName = NULL);

  virtual
   ~OsMulticastSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

     /// Joins a multicast group. Returns 0 on success.
   int joinGroup(const char* multicastHostName);

   //int leaveGroup(const char* multicastHostName);

     /// Sets the hop count (a.k.a. TimeToLive) for outgoing multicast packets.
   int setHopCount(unsigned char hopCount);

     /// Enables/disables local loopback of outgoing multicast packets.
   int setLoopback(bool enabled);

/* ============================ ACCESSORS ================================= */
   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMulticastSocket(const OsMulticastSocket& rOsMulticastSocket);
     //:Disable copy constructor

   OsMulticastSocket& operator=(const OsMulticastSocket& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMulticastSocket_h_
