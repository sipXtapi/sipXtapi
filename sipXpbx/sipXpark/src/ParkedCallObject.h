// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _ParkedCallObject_h_
#define _ParkedCallObject_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/CallManager.h>
#include <mp/MpStreamPlayer.h>

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
class ParkedCallObject
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ParkedCallObject(CallManager* callManager, UtlString callId, UtlString playFile);
   ~ParkedCallObject();

   void setAddress(UtlString address);
   UtlString getAddress();

   OsStatus playAudio();

   void cleanUp();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CallManager* mpCallManager;
    UtlString mCallId;
    UtlString mAddress;
    
    MpStreamPlayer* mpPlayer;
    UtlString mFile;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ParkedCallObject_h_
