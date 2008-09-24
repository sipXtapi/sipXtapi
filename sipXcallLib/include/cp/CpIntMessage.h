//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _CpIntMessage_h_
#define _CpIntMessage_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CallManager.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpIntMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    CpIntMessage(unsigned char messageSubtype = CallManager::CP_UNSPECIFIED,
       const int intValue = 0);
     //:Default constructor


   virtual
   ~CpIntMessage();
     //:Destructor

   virtual OsMsg* createCopy(void) const;

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
        void getIntData(int& intValue) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        int mIntData;

   CpIntMessage(const CpIntMessage& rCpIntMessage);
     //:disable Copy constructor

   CpIntMessage& operator=(const CpIntMessage& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpIntMessage_h_
