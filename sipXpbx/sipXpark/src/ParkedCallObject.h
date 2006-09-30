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
#include <os/OsTimer.h>
#include <os/OsMsgQ.h>
#include <OrbitFileReader.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//: Object to describe and control a parked call.
//  All methods are executed by the thread of the owning OrbitListener.
class ParkedCallObject
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // Enum to differentiate NOTIFY messages.
    // Max value must be less than sSeqNoIncrement.
    enum notifyCodes
       {
          DTMF,
          TIMEOUT
       };

/* ============================ CREATORS ================================== */

   ParkedCallObject(const UtlString& orbit,
                    CallManager* callManager,
                    const UtlString& callId, 
                    const UtlString& playFile,
                    bool bPickup,
                    OsMsgQ* listenerQ);
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
   
   // Set up the "escape from parking orbit" mechanisms:
   // If a parker URI and timeout are supplied, set a timer to trigger
   // a transfer to the parker.
   // If a parker URI and DTMF code are supplied, set a DTMF listener
   // to allow the user to force a transfer to the parker.
   void startEscapeTimer(UtlString& parker,
                         ///< URI that parked this call, or "".
                         int timeout,
                         ///< seconds for timeout, or OrbitData::NO_TIMEOUT
                         int keycode
                         /**< RFC 2833 keycode to escape from park, or
                          *   OrbitData::NO_KEYCODE for none.
                          */
      );

   // Stop the time-out timer.

   // The escape mechanisms are usually cancelled automatically by the
   // ParkedCallObject::~, and since their notifications are done via
   // messages to the OrbitListener that contain only mSeqNo, race
   // conditions are not a problem.  But when a dialog is replaced by
   // another dialog, we need to stop the escape timer on the ParkedCallObject
   // for the first dialog.
   void stopEscapeTimer();

   // Initiate a blind transfer of this call to mParker.
   void startTransfer();

   // Signal that a transfer attempt for a call has ended.
   void clearTransfer();

   // Process a DTMF keycode.
   void keypress(int keycode);

   // Split a userData value into the seqNo and "enum notifyCodes".
   static void splitUserData(int userData, int& seqNo, enum notifyCodes& type)
      {
         seqNo = userData & sSeqNoMask;
         type = (enum notifyCodes) (userData & ~seqNo);
      };

   // Get the seqNo.
   int getSeqNo()
      {
         return mSeqNo;
      };

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // Sequence number of this ParkedCallObject.
    int mSeqNo;
    // Next seqNo to be assigned.
    static int sNextSeqNo;
    // Amount to increment successive seqNo's.
    // Must be lowest 1 bit of seqNoMask.
    static const int sSeqNoIncrement;
    // Mask to cause seqNo's to wrap around.
    static const int sSeqNoMask;

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

    // Members to support the transfer back to parker feature.
    UtlString mParker;          ///< The URI of the user that parked the call.
    OsTimer mTimeoutTimer;      ///< OsTimer to trigger the timeout.
    // Support for processing DTMF events.
    OsQueuedEvent mDtmfEvent;
    int mKeycode;               /**< keycode to transfer back, or
                                 *   OrbitData::NO_KEYCODE */
    /// Set to TRUE if a transfer to the parker is in progress.
    //  Used to ensure that a transfer is not started if one is already
    //  started.
    UtlBoolean mTransferInProgress;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ParkedCallObject_h_
