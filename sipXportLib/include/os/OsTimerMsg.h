//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTimerMsg_h_
#define _OsTimerMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsRpcMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;

//:Messages used to request timer services

class OsTimerMsg : public OsRpcMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgSubType
   {
      UNSPECIFIED,      // not yet initialized
      START,            // start a timer
      STOP              // cancel a timer
   };
     //!enumcode: UNSPECIFIED - not yet initialized
     //!enumcode: START - start a timer
     //!enumcode: STOP  - cancel a timer

/* ============================ CREATORS ================================== */

   OsTimerMsg(const unsigned char subType, OsEvent& rEvent, OsTimer& rTimer);
     //:Constructor

   OsTimerMsg(const OsTimerMsg& rOsTimerMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~OsTimerMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTimerMsg& operator=(const OsTimerMsg& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   virtual OsTimer* getTimer(void) const;
     //:Return a pointer to the OsTimer object associated with this message

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsTimer* mpTimer;

   void init(void);
     //:Initialization common to all constructors

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerMsg_h_
