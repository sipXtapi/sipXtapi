//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "CallStateEventBuilder_XML.h"

#include "os/OsSysLog.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define PRETTYPRINT_EVENTS 
#ifdef PRETTYPRINT_EVENTS
#  define PP_LF "\n"
#  define PP_IN "  "
#else
#  define PP_LF ""
#  define PP_IN ""
#endif

const char* CallEventElementStartFormat =
"<call_event xmlns='http://www.sipfoundry.org/sipX/schema/xml/cse-01-00'>" PP_LF
PP_IN "<observer>%s</observer>" PP_LF
PP_IN "<obs_seq>%ld</obs_seq>" PP_LF
;
const char* ObsTimeElementFormat =
PP_IN "<obs_time>TBD</obs_time>" PP_LF
;
const char* CallEventElementEnd =
"</call_event>" PP_LF
;

const char* ObsMsgFormat =
PP_IN "<obs_msg>" PP_LF
PP_IN PP_IN "<obs_status>%ld</obs_status>" PP_LF
PP_IN PP_IN "<obs_text>%s</obs_text>" PP_LF
PP_IN "</obs_msg>" PP_LF
;

const char* CallRequestElementStart =
PP_IN "<call_request>" PP_LF
;
const char* CallRequestElementEnd =
PP_IN "</call_request>" PP_LF
;

const char* CallElementFormat =
PP_IN PP_IN "<call>" PP_LF
PP_IN PP_IN PP_IN "<dialog>" PP_LF
PP_IN PP_IN PP_IN PP_IN "<call_id>%s</call_id>" PP_LF
PP_IN PP_IN PP_IN PP_IN "<from_tag>%s</from_tag>" PP_LF
PP_IN PP_IN PP_IN PP_IN "<to_tag>%s</to_tag>" PP_LF
PP_IN PP_IN PP_IN "</dialog>" PP_LF
PP_IN PP_IN PP_IN "<from>%s</from>" PP_LF
PP_IN PP_IN PP_IN "<to>%s</to>" PP_LF
PP_IN PP_IN "</call>" PP_LF
;

const char* ContactElementFormat =
PP_IN PP_IN "<contact>%s</contact>" PP_LF
;

const char* ViaElementFormat =
PP_IN PP_IN "<via>%s</via>" PP_LF
;

#define CONTENT_BUF_MAX 2048 

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventBuilder_XML::CallStateEventBuilder_XML(const char* observerDnsName ///< the DNS name to be recorded in all events
                                                     ) :
   CallStateEventBuilder(observerDnsName)
{
}


/// Destructor
CallStateEventBuilder_XML::~CallStateEventBuilder_XML()
{
}


/**
 * Generate a metadata event.
 * This method generates a complete event - it does not require that the callEventComplete method be called.
 */
void CallStateEventBuilder_XML::observerEvent(int sequenceNumber, ///< for ObserverReset, this should be zero
                                              int timestamp,      ///< UTC in seconds since the unix epoch
                                              ObserverEvent eventCode,
                                              const char* eventMsg ///< for human consumption
                                              )
{
   BuilderMethod specialEvent;
   switch (eventCode)
   {
   case ObserverReset:
      specialEvent = BuilderStart;
      break;
   case ObserverError:
      specialEvent = BuilderReset;
      break;
   default:
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "observerEvent: invalid eventCode %d", eventCode);
      break;
   }

   if (builderStateIsOk(specialEvent))
   {
      
   }
   else
   {
      reset();
   }
}



/// Begin a Call Request Event - an INVITE without a to tag has been observed
/**
 * Requires:
 *   - callRequestEvent
 *   - addCallData (the toTag in the addCallRequest will be a null string)
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callRequestEvent(int sequenceNumber,
                                                 int timestamp,
                                                 const UtlString& contact
                                                 )
{
}


/// Begin a Call Setup Event - a 2xx response to an INVITE has been observed
/**
 * Requires:
 *   - callSetupEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callSetupEvent(int sequenceNumber,
                                               int timestamp,
                                               const UtlString& contact
                                               )
{
}


/// Begin a Call Failure Event - an error response to an INVITE has been observed
/**
 * Requires:
 *   - callFailureEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callFailureEvent(int sequenceNumber,
                                                 int timestamp,
                                                 int statusCode,
                                                 const UtlString& statusMsg
                                                 )
{
}


/// Begin a Call End Event - a BYE request has been observed
/**
 * Requires:
 *   - callEndEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callEndEvent(const int sequenceNumber,
                                             const int timestamp
                                             )
{
}


/// Add the dialog and call information for the event being built.
void CallStateEventBuilder_XML::addCallData(const UtlString& callId,
                                            const UtlString& fromTag,  /// may be a null string
                                            const UtlString& toTag,    /// may be a null string
                                            const UtlString& fromField,
                                            const UtlString& toField
                                            )
{
}

   
/// Add a via element for the event
/**
 * Record the specified Via from the message for this event, where 0
 * indicates the Via inserted by the message originator.  At least that
 * via should be added for any event.
 */
void CallStateEventBuilder_XML::addEventVia(int index,
                                            const UtlString& via
                                            )
{
}


/// Indicates that all information for the current call event has been added.
void CallStateEventBuilder_XML::completeCallEvent()
{
}

/// Indicates that all information for the current call event has been added.
void CallStateEventBuilder_XML::reset()
{
   mCurrentEvent.remove(0);
   mViaList.destroyAll();
   mLaterElement.remove(0);
}



