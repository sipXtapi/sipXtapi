// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */
/** 
 * @file sipXtapiEvents.h
 * sipXtapi event declarations
 *
 * The sipXtapiEvents.h header file defines all of the events fired as part of
 * receiving inbound calls and placing outbound calls.  Each event notification 
 * is comprised of a major event and a minor event.  The major event identifies 
 * a significant state transition (e.g. from connected to disconnected.  The minor 
 * event identifies the reason for the change (cause code).
 *
 * Below you will find state diagrams that show the typical event transitions 
 * for both outbound and inbound calls.
 *
 * @image html callevents_inbound.gif
 *
 * Figure 1: Event flows for an inbound call (received call)
 *
 * @image html callevents_outbound.gif
 *
 * Figure 2: Event flows for an outbound call (placed call)
 */
#ifndef _SIPXTAPIEVENT_H
#define _SIPXTAPIEVENT_H


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "sipXtapi.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS

// TYPEDEFS

/**
 * Major call state events identify significant changes in the state of a 
 * call.
 */
typedef enum SIPX_CALLSTATE_MAJOR
{
    UNKNOWN         = 0,    /**< An UNKNOWN event is generated when the state for a call 
                                 is no longer known.  This is generally an error 
                                 condition; see the minor event for specific causes. */
    NEWCALL		    = 1000, /**< The NEWCALL event indicates that a new call has been 
                                 created automatically by the sipXtapi.  This event is 
                                 most frequently generated in response to an inbound 
                                 call request.  */
	DIALTONE        = 2000, /**< The DIALTONE event indicates that a new call has been 
                                 created for the purpose of placing an outbound call.  
                                 The application layer should determine if it needs to 
                                 simulate dial tone for the end user. */
	REMOTE_OFFERING = 2500, /**< The REMOTE_OFFERING event indicates that a call setup 
                                 invitation has been sent to the remote party.  The 
                                 invitation may or may not every receive a response.  If
                                 a response is not received in a timely manor, sipXtapi 
                                 will move the call into a disconnected state.  If 
                                 calling another sipXtapi user agent, the reciprocate 
                                 state is OFFER. */
	REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup 
                                 invitation has been accepted and the end user is in the
                                 alerting state (ringing).  Depending on the SIP 
                                 configuration, end points, and proxy servers involved, 
                                 this event should only last for 3 minutes.  Afterwards,
                                 the state will automatically move to DISCONNECTED.  If 
                                 calling another sipXtapi user agent, the reciprocate 
                                 state is ALERTING. */
	CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup 
                                 between the local and remote party.  Audio should be 
                                 flowing provided and the microphone and speakers should
                                 be engaged. */
	DISCONNECTED    = 5000, /**< The DISCONNECTED state indicates that a call was 
                                 disconnected or failed to connect.  A call may move 
                                 into the DISCONNECTED states from almost every other 
                                 state.  Please review the DISCONNECTED minor events to
                                 understand the cause. */
	OFFERING        = 6000, /**< An OFFERING state indicates that a new call invitation 
                                 has been extended this user agent.  Application 
                                 developers should invoke sipxCallAccept(), 
                                 sipxCallReject() or sipxCallRedirect() in response.  
                                 Not responding will result in an implicit call 
                                 sipXcallReject(). */
	ALERTING        = 7000, /**< An ALERTING state indicates that an inbound call has 
                                 been accepted and the application layer should alert 
                                 the end user.  The alerting state is limited to 3 
                                 minutes in most configurations; afterwards the call 
                                 will be canceled.  Applications will generally play 
                                 some sort of ringing tone in response to this event. */
	DESTROYED       = 8000, /**< The DESTORYED event indicates the underlying resources 
                                 have been removed for a call.  This is the last event 
                                 that the application will receive for any call.  The 
                                 call handle is invalid after this event is received. */
} SIPX_CALLSTATE_MAJOR ;


/**
 * Minor call events identify the reason for a SIPX_CALLSTATE_MAJOR event or 
 * provide more detail.
 */
typedef enum SIPX_CALLSTATE_MINOR
{
	NEW_CALL_NORMAL			= NEWCALL + 1,	        /**< See NEWCALL major event */
	DIALTONE_UNKNOWN        = DIALTONE + 1,         /**< See DIALTONE major event */
    DIALTONE_CONFERENCE     = DIALTONE,             /**< Call created as part of conference */
	REMOTE_OFFERING_NORMAL  = REMOTE_OFFERING + 1,  /**< See REMOTE_OFFERING major event */
	REMOTE_ALERTING_NORMAL  = REMOTE_ALERTING + 1,  /**< Remote party is alerting, play ringback 
                                                         locally */
	REMOTE_ALERTING_MEDIA,                          /**< Remote party is alerting and providing 
                                                         ringback audio*/
	CONNECTED_ACTIVE		= CONNECTED + 1,        /**< Call is connected and active (playing 
                                                         local media)*/
	CONNECTED_ACTIVE_HELD,                          /**< Call is connected, held (not playing local
                                                         media), and bridging media for a 
                                                         conference */
    CONNECTED_INACTIVE,                             /**< Call is held (not playing local media) and
                                                         is not bridging any other calls */
	DISCONNECTED_BADADDRESS = DISCONNECTED + 1,     /**< Disconnected: Invalid or unreachable 
                                                         address */
	DISCONNECTED_BUSY,                              /**< Disconnected: Caller or Callee was busy*/
	DISCONNECTED_NORMAL,                            /**< Disconnected: Normal call tear down (either 
                                                         local or remote)*/
    DISCONNECTED_RESOURCES,                         /**< Disconnected: Not enough resources 
                                                         available to complete call*/
    DISCONNECTED_NETWORK,                           /**< Disconnected: A network error cause call 
                                                         to fail*/
	DISCONNECTED_REDIRECTED,                        /**< Disconnected: Call was redirected a 
                                                         different user agent */
	DISCONNECTED_NO_RESPONSE,                       /**< Disconnected: No response was received */
    DISCONNECTED_AUTH,                              /**< Disconnected: Unable to authenticate */
    DISCONNECTED_UNKNOWN,                           /**< Disconnected: Unknown reason */

	OFFERING_ACTIVE		    = OFFERING + 1,         /**< See OFFERING major event */
	ALERTING_NORMAL			= ALERTING + 1,         /**< See ALERTING major event */
	DESTROYED_NORMAL        = DESTROYED + 1,        /**< See DESTROYED major event */

} SIPX_CALLSTATE_MINOR ;


/**
 * Signature for event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @param hCall Source of event
 * @param hLine Line call was made on
 * @param eMajor Major event ID
 * @param eMinor Minor event ID
 * @param pUserData User data provided when listener was added
 */
typedef void (*CALLBACKPROC)( SIPX_CALL hCall, 
						      SIPX_LINE hLine, 
						      SIPX_CALLSTATE_MAJOR eMajor, 
						      SIPX_CALLSTATE_MINOR eMinor, 
						      void* pUserData) ;
						      

/* ============================ FUNCTIONS ================================= */

/**
 * Add a callback/observer for the purpose of receiving call events
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxListenerAdd(const SIPX_INST hInst, 
                                         CALLBACKPROC pCallbackProc, 
                                         void* pUserData) ;

/**
 * Remove a call event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxListenerAdd.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxListenerRemove(const SIPX_INST hInst, 
                                            CALLBACKPROC pCallbackProc, 
                                            void* pUserData) ;


/**
 * Create a printable string version of the designated call state event ids.
 * This is generally used for debugging.
 *
 * @param eMajor sipxtapi major event code
 * @param eMinor sipxtapi minor event code
 */
SIPXTAPI_API char* sipxEventToString(SIPX_CALLSTATE_MAJOR eMajor, SIPX_CALLSTATE_MINOR eMinor) ;

/**
 * Enumeration of possible Line Events.
 */
typedef enum SIPX_LINE_EVENT_TYPE_MAJOR
{
   SIPX_LINE_EVENT_UNKNOWN          = -1,       /**< This is the initial Line event state. */
   SIPX_LINE_EVENT_REGISTERING      = 10000,    /**< The REGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_REGISTERED       = 11000,    /**< The REGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     registration. */
   SIPX_LINE_EVENT_UNREGISTERING    = 12000,    /**< The UNREGISTERING event is fired when sipXtapi
                                                     has successfully sent a REGISTER message with an expires=0 parameter,
                                                     but has not yet received a success response from the
                                                     registrar server */
   SIPX_LINE_EVENT_UNREGISTERED     = 13000,    /**< The UNREGISTERED event is fired after sipXtapi has received
                                                     a response from the registrar server, indicating a successful
                                                     un-registration. */
   SIPX_LINE_EVENT_REGISTER_FAILED  = 14000,    /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                                     It is fired in the following cases:  
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_UNREGISTER_FAILED  = 15000, /**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                                     It is fired in the following cases:  
                                                     The client could not connect to the registrar server.
                                                     The registrar server challenged the client for authentication credentials,
                                                     and the client failed to supply valid credentials.
                                                     The registrar server did not generate a success response (status code == 200)
                                                     within a timeout period.  */
   SIPX_LINE_EVENT_PROVISIONED      = 16000    /**<  The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not 
                                                     requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */ 
} SIPX_LINE_EVENT_TYPE_MAJOR;

/**
 * Signature for line event callback/observer.  Application developers should
 * not block the calling thread.
 *
 * @param hLine Line call was made on
 * @param eMajor Major event ID
 * @param eMinor Minor event ID
 * @param pUserData User data provided when listener was added
 */
typedef void (*LINECALLBACKPROC)(SIPX_LINE hLine, 
						      SIPX_LINE_EVENT_TYPE_MAJOR eMajor, 
						      void* pUserData);

/**
 * Add a callback/observer for the purpose of receiving Line events
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxLineListenerAdd(const SIPX_INST hInst, 
                                         LINECALLBACKPROC pCallbackProc, 
                                         void* pUserData);

/**
 * Remove a Line event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxLineListenerAdd.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx Line events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxLineListenerRemove(const SIPX_INST hInst, 
                                            LINECALLBACKPROC pCallbackProc, 
                                            void* pUserData);
                                            
/**
 * Create a printable string version of the designated line event ids.
 * This is generally used for debugging.
 *
 * @param lineTypeMajor major event type id
 * @param lineTypeMinor minor event type id
 */
SIPXTAPI_API char* sipxLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR lineTypeMajor);                                            



#endif

#endif /* ] SIPXTAPI_EXCLUDE */
