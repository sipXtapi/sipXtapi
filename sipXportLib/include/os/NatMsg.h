// 
// Copyright (C) 2006 Pingtel Corp.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _NatMsg_h_
#define _NatMsg_h_

#include "os/IStunSocket.h"
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
          IStunSocket*  pSocket, 
          UtlString     receivedIp, 
          int           iReceivedPort);
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

   IStunSocket* getSocket() const ;

   UtlString getReceivedIp() const ;

   int getReceivedPort() const ;

   int getType() const ;

   void* getContext() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    int           miType ;
    char*         mBuffer ;
    int           mLength ;
    IStunSocket*  mpSocket ;
    UtlString     mReceivedIp ;
    int           miReceivedPort ;
    void*         mpContext ;
    


/* //////////////////////////// PRIVATE /////////////////////////////////// */
};

#endif
