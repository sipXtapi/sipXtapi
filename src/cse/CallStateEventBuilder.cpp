//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "assert.h"

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "CallStateEventBuilder.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

enum BuildStates // used in builderStateOk
{
   Initial          = 0x0000, 
   BetweenEvents    = 0x0001,
   CallEventStarted = 0x0002, /* --\                                                 */
   CallDataAdded    = 0x0004, /*  these three are ORed together to form combinations */
   ViaAdded         = 0x0008, /* --/                                                 */
   Error            = 0x8000
};

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventBuilder::CallStateEventBuilder(const char* observerDnsName ///< the DNS name to be recorded in all events
                                             ) :
   buildState(Initial)
{
   assert(NULL!=observerDnsName);

   observerName = strdup(observerDnsName);
}


/// Destructor
CallStateEventBuilder::~CallStateEventBuilder()
{
   assert(NULL!=observerName);

   free(const_cast<char*>(observerName));
}


/**
 * Generate a metadata event.
 * This method generates a complete event - it does not require that the callEventComplete method be called.
 */
void CallStateEventBuilder::observerEvent(int sequenceNumber, ///< for ObserverReset, this should be zero
                                          int timestamp,      ///< UTC in seconds since the unix epoch
                                          ObserverEvent eventCode,
                                          const char* eventMsg ///< for human consumption
                                          )
{
   bool CalledBaseClassMethod_observerEvent = false;
   assert(CalledBaseClassMethod_observerEvent);
}


/// Begin a Call Request Event - an INVITE without a to tag has been observed
/**
 * Requires:
 *   - callRequestEvent
 *   - addCallData (the toTag in the addCallRequest will be a null string)
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder::callRequestEvent(int sequenceNumber,
                                             int timestamp,
                                             const UtlString& contact
                                             )
{
   bool CalledBaseClassMethod_callRequestEvent = false;
   assert(CalledBaseClassMethod_callRequestEvent);
}


/// Begin a Call Setup Event - a 2xx response to an INVITE has been observed
/**
 * Requires:
 *   - callSetupEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder::callSetupEvent(int sequenceNumber,
                                           int timestamp,
                                           const UtlString& contact
                                           )
{
   bool CalledBaseClassMethod_callSetupEvent = false;
   assert(CalledBaseClassMethod_callSetupEvent);
}


/// Begin a Call Failure Event - an error response to an INVITE has been observed
/**
 * Requires:
 *   - callFailureEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder::callFailureEvent(int sequenceNumber,
                                             int timestamp,
                                             int statusCode,
                                             const UtlString& statusMsg
                                             )
{
   bool CalledBaseClassMethod_callFailureEvent = false;
   assert(CalledBaseClassMethod_callFailureEvent);
}


/// Begin a Call End Event - a BYE request has been observed
/**
 * Requires:
 *   - callEndEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder::callEndEvent(const int sequenceNumber,
                                         const int timestamp
                                         )
{
   bool CalledBaseClassMethod_callEndEvent = false;
   assert(CalledBaseClassMethod_callEndEvent);
}


/// Add the dialog and call information for the event being built.
void CallStateEventBuilder::addCallData(const UtlString& callId,
                                        const UtlString& fromTag,  /// may be a null string
                                        const UtlString& toTag,    /// may be a null string
                                        const UtlString& fromField,
                                        const UtlString& toField
                                        )
{
   bool CalledBaseClassMethod_addCallData = false;
   assert(CalledBaseClassMethod_addCallData);
}

   
/// Add a via element for the event
/**
 * Record the specified Via from the message for this event, where 0
 * indicates the Via inserted by the message originator.  At least that
 * via should be added for any event.
 */
void CallStateEventBuilder::addEventVia(int index,
                                        const UtlString& via
                                        )
{
   bool CalledBaseClassMethod_addEventVia = false;
   assert(CalledBaseClassMethod_addEventVia);
}


/// Indicates that all information for the current call event has been added.
void CallStateEventBuilder::completeCallEvent()
{
   bool CalledBaseClassMethod_completeCallEvent = false;
   assert(CalledBaseClassMethod_completeCallEvent);
}


/// Event validity checking state machine
/**
 * Each of the public builder methods calls this finite state machine to determine
 * whether or not the call is valid.  This allows all builders to share the same
 * rules for what calls are allowed and required when constructing each event type
 * (for this reason, this should not be redefined by derived classes).
 *
 * In the initial state, the only valid call is builderStateOk(ObserverStart)
 *
 * @returns
 *   - true if method is valid now
 *   - false if method is not valid now
 *
 * If this routine returns false, the caller should clear all saved state and then
 * call this routine again, passing ObserverReset.  The finite state machine is then
 * reset to the initial state.
 *
 * This state machine is defined by the valid call state events specified
 * in the meta/call_event.xsd schema.
 */
bool CallStateEventBuilder::builderStateIsOk( BuilderMethod method )
{
   bool isValid = false; // pessimism is safe
   
#  define VALID(newState) { isValid = true; buildState = (newState); }
#  define INVALID         { buildState = Error; }
   
   if (BuilderStart == method)
   {
      VALID(BetweenEvents);
   }
   else if (BuilderReset == method)
   {
      VALID(Initial);
   }
   else
   {
      switch (buildState)
      {
      case Initial:
         // only BuilderStart is valid, which was handled above
         INVALID;
         break;

      case BetweenEvents:
         switch (method)
         {
         case CallRequestEvent:
         case CallSetupEvent:
         case CallFailureEvent:
         case CallEndEvent:
            VALID(CallEventStarted);
            break;
         default:
            INVALID;
         }
      
      case CallEventStarted:
      case CallEventStarted|ViaAdded:
         switch (method)
         {
         case AddCallData:
            VALID(buildState|CallDataAdded);
            break;
         case AddVia:
            VALID(buildState|ViaAdded);
            break;
         default:
            INVALID;
         }
      
      case CallEventStarted|CallDataAdded:
         switch (method)
         {
         case AddVia:
            VALID(buildState|ViaAdded);
            break;
         default:
            INVALID;
         }

      case CallEventStarted|CallDataAdded|ViaAdded:
         switch (method)
         {
         case AddVia:
            VALID(buildState);
            break;
         case CompleteCallEvent:
            VALID(BetweenEvents);
            break;
         default:
            INVALID;
         }

      case Error:
         // we returned false on the last call
         // the caller must acknowlege the error with a BuilderReset
         INVALID;  // the valid call was handled outside the switch
         break;
         
      default:
      {
         bool InvalidBuilderState = false;
         assert(InvalidBuilderState);
         INVALID;
      }
      }
   }

   return isValid;
}
      


