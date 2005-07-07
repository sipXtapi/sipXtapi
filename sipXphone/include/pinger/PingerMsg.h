// $Id: //depot/OPENDEV/sipXphone/include/pinger/PingerMsg.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PingerMsg_h_
#define _PingerMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Message object used to communicate phone set information
class PingerMsg : public OsMsg  
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // MsgType categories defined for use by the system
   enum PingerMsgTypes
   {
      UNSPECIFIED = 0,
      PLAY_SPLASH,
      START_SIP_UA,
      START_SIP_REFRESH_MGR,
      START_SIP_LINE_MGR,
      JAVA_INITIALIZED,
      WAIT_FOR_SIP_UA
   };

/* ============================ CREATORS ================================== */

   PingerMsg(int msgSubType, const char* pStrParam = NULL,
             const int intParam = 0);

   PingerMsg(const PingerMsg& rPingerMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual ~PingerMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PingerMsg& operator=(const PingerMsg& rhs);
     //:Assignment operator

   void setStringParam(const char* strData);

   void setIntParam(int data);

/* ============================ ACCESSORS ================================= */

   void getStringParam(UtlString& rStringData);

   int getIntParam();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int   mIntParam;      // Integer parameter
   char* mpStringParam;  // String parameter

}; 

#endif // _PingerMsg_h_
