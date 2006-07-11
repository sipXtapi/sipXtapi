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
#include <os/OsDateTime.h>
#include <utl/UtlSList.h>

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

   ParkedCallObject(UtlString& orbit, CallManager* callManager, UtlString callId, 
                    UtlString playFile, bool bPickup=false);
   ~ParkedCallObject();

   void setAddress(UtlString address);
   UtlString getAddress();
   
   // Get or set a callId that is the retrieving call
   UtlString getPickupCallId();
   void setPickupCallId(const char *callId);
   
   // Get or set the new callId for the second leg of a transfer
   UtlSList* getNewCallIds();
   void setNewCallId(const char* callId);
   
   UtlString getOrbit();
   void getTimeParked(OsTime& parked);
   void setOriginalAddress(UtlString& address);
   UtlString getOriginalAddress();
   
   bool isPickupCall();
   bool hasNewCallIds();

   OsStatus playAudio();
   
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
    UtlString mPickupCallId;
    UtlString mNewCallId;
    UtlString mOrbit;
    UtlString mOriginalAddress;
    
    // Remember all tranferred in calls in this list for clean-up
    UtlSList mNewCallIds;    
    
    bool mbPickup;             // Call is a retrieval call
    
    OsTime mParked;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ParkedCallObject_h_
