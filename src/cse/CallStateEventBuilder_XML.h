//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventBuilder_XML_h_
#define _CallStateEventBuilder_XML_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "CallStateEventBuilder.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This CallStateEventBuilder constructs events as XML elements according
 * to the specification doc/cdr/call-state-events.html
 *
 * for usage of the event generation interfaces, @see CallStateEventBuilder
 */
class CallStateEventBuilder_XML : public CallStateEventBuilder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   CallStateEventBuilder_XML(const char* observerDnsName ///< the DNS name to be recorded in all events
                             );

   /// Destructor
   ~CallStateEventBuilder_XML();

   /**
    * Generate a metadata event.
    * This method generates a complete event - it does not require that the callEventComplete method be called.
    */
   void observerEvent(int sequenceNumber, ///< for ObserverReset, this should be zero
                      int timestamp,      ///< UTC in seconds since the unix epoch
                      ObserverEvent eventCode,
                      const char* eventMsg ///< for human consumption
                      );

   /// Begin a Call Request Event - an INVITE without a to tag has been observed
   /**
    * Requires:
    *   - callRequestEvent
    *   - addCallData (the toTag in the addCallRequest will be a null string)
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */
   void callRequestEvent(int sequenceNumber,
                         int timestamp,
                         const UtlString& contact
                         );

   /// Begin a Call Setup Event - a 2xx response to an INVITE has been observed
   /**
    * Requires:
    *   - callSetupEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */
   void callSetupEvent(int sequenceNumber,
                       int timestamp,
                       const UtlString& contact
                       );

   /// Begin a Call Failure Event - an error response to an INVITE has been observed
   /**
    * Requires:
    *   - callFailureEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */
   void callFailureEvent(int sequenceNumber,
                         int timestamp,
                         int statusCode,
                         const UtlString& statusMsg
                         );

   /// Begin a Call End Event - a BYE request has been observed
   /**
    * Requires:
    *   - callEndEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */
   void callEndEvent(const int sequenceNumber,
                     const int timestamp
                     );

   /// Add the dialog and call information for the event being built.
   void addCallData(const UtlString& callId,
                    const UtlString& fromTag,  /// may be a null string
                    const UtlString& toTag,    /// may be a null string
                    const UtlString& fromField,
                    const UtlString& toField
                    );
   
   /// Add a via element for the event
   /**
    * Record the specified Via from the message for this event, where 0
    * indicates the Via inserted by the message originator.  At least that
    * via should be added for any event.
    */
   void addEventVia(int index,
                    const UtlString& via
                    );

   /// Indicates that all information for the current call event has been added.
   void completeCallEvent();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   UtlString mCurrentEvent;
   UtlSList  mViaList;
   UtlString mLaterElement;

   void reset();
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventBuilder_XML_h_

