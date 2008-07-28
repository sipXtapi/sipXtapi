// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _NatMsg_h_
#define _NatMsg_h_

#include "os/IOsNatSocket.h"
#include "os/OsMsg.h"
#include "utl/UtlString.h"


//: Basic NatMsg -- relies on external bodies to allocate and free memory.
class NatMsg : public OsMsg
{
public:

enum
{
    STUN_MESSAGE,
    TURN_MESSAGE,
    EXPIRATION_MESSAGE
} ;

/* ============================ CREATORS ================================== */
   NatMsg(int           type,
          char*         szBuffer, 
          int           nLength, 
          IOsNatSocket*  pSocket,
          UtlString     receivedIp, 
          int           iReceivedPort,
          UtlString*    pRelayIp = NULL,
          int*          pRelayPort = NULL);
     //:Constructor
   
   NatMsg(int   type,
          void* pContext);
     //:Constructor


   NatMsg(const NatMsg& rNatMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~NatMsg();
     //:Destructor
/* ============================ MANIPULATORS ============================== */

   NatMsg& operator=(const NatMsg& rhs);
     //:Assignment operator
/* ============================ ACCESSORS ================================= */

   char* getBuffer() const ;

   int getLength() const ;

   IOsNatSocket* getSocket() const ;

   UtlString getReceivedIp() const ;

   int getReceivedPort() const ;

   int getType() const ;

   void* getContext() const ;

   UtlString getRelayIp() const ;

   int getRelayPort() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    int           miType ;
    char*         mBuffer ;
    int           mLength ;
    IOsNatSocket* mpSocket ;
    UtlString     mReceivedIp ;
    int           miReceivedPort ;
    void*         mpContext ;
    UtlString     mRelayIp ;
    int           miRelayPort ;
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
};

#endif
