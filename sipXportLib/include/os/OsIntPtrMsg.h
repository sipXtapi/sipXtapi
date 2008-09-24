//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef OsIntPtrMsg_h__
#define OsIntPtrMsg_h__

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

class OsIntPtrMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */

   /* ============================ CREATORS ================================== */

   OsIntPtrMsg(const unsigned char msgType, 
      const unsigned char msgSubType, 
      intptr_t pData1 = 0,
      intptr_t pData2 = 0);
   //:Constructor

   OsIntPtrMsg(const OsIntPtrMsg& rOsMsg);
   //:Copy constructor

   virtual OsMsg* createCopy(void) const;
   //:Create a copy of this msg object (which may be of a derived type)

   /* ============================ MANIPULATORS ============================== */

   OsIntPtrMsg& operator=(const OsIntPtrMsg& rhs);
   //:Assignment operator

   /* ============================ ACCESSORS ================================= */
   intptr_t getData1() const { return mpData1; }
   intptr_t getData2() const { return mpData2; }

   void setData1(intptr_t val) { mpData1 = val; }
   void setData2(intptr_t val) { mpData2 = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   intptr_t mpData1;
   intptr_t mpData2;

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif // OsIntPtrMsg_h__
