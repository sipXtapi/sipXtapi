//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsPtrMsg_h_
#define _OsPtrMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsMsg.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for message queue buffers

class OsPtrMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsPtrMsg(const unsigned char msgType, const unsigned char msgSubType, void* pData);
     //:Constructor

   OsPtrMsg(const OsPtrMsg& rOsMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

/* ============================ MANIPULATORS ============================== */

   OsPtrMsg& operator=(const OsPtrMsg& rhs);
     //:Assignment operator


/* ============================ ACCESSORS ================================= */
   void* OsPtrMsg::getPtr();
   

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */
   void* mpData;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsPtrMsg_h_
