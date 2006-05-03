//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


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

#ifdef SIPX_USE_STDCALL
#define SIPX_CALLING_CONVENTION __stdcall
#else
#define SIPX_CALLING_CONVENTION
#endif 

/**
 * Enum with all of the possible event types.
 */
typedef enum SIPX_EVENT_CATEGORY
{
    EVENT_CATEGORY_CALLSTATE,       /**< CALLSTATE events signify a change in state of a 
                                         call.  States range from the notification of a 
                                         new call to ringing to connection established to 
                                         changes in audio state (starting sending, stop 
                                         sending) to termination of a call. */
    EVENT_CATEGORY_LINESTATE,       /**< LINESTATE events indicate changes in the status 
                                         of a line appearance.  Lines identify inbound 
                                         and outbound identities and can be either 
                                         provisioned (hardcoded) or configured to 
                                         automatically register with a registrar.  
                                         Lines also encapsulate the authentication 
                                         criteria needed for dynamic registrations. */
    EVENT_CATEGORY_INFO_STATUS,     /**< INFO_STATUS events are sent when the application 
                                         requests sipXtapi to send an INFO message to 
                                         another user agent.  The status event includes 
                                         the response for the INFO method.  Application 
                                         developers should look at this event to determine 
                                         the outcome of the INFO message. */
    EVENT_CATEGORY_INFO,            /**< INFO events are sent to the application whenever 
                                         an INFO message is received by the sipXtapi user 
                                         agent.  INFO messages are sent to a specific call.
                                         sipXtapi will automatically acknowledges the INFO 
                                         message at the protocol layer. */
    EVENT_CATEGORY_SUB_STATUS,      /**< SUB_STATUS events are sent to the application 
                                         layer for information on the subscription state
                                         (e.g. OK, Expired). */                                    
    EVENT_CATEGORY_NOTIFY,          /**< NOTIFY evens are send to the application layer
                                         after a remote publisher has sent data to the 
                                         application.  The application layer can retrieve
                                         the data from this event. */
    EVENT_CATEGORY_CONFIG,          /**< CONFIG events signify changes in configuration.
                                         For example, when requesting STUN support, a 
                                         notification is sent with the STUN outcome (either
                                         SUCCESS or FAILURE) */
    EVENT_CATEGORY_SECURITY,        /**< SECURITY events signify occurences in call security 
                                         processing.  These events are only sent when using
                                         S/MIME or TLS. */
    EVENT_CATEGORY_MEDIA            /**< MEDIA events signify changes in the audio state for
                                         sipXtapi or a particular call. */
                                         
} SIPX_EVENT_CATEGORY;

#define VALID_SIPX_EVENT_CATEGORY(x) (((x) >= EVENT_CATEGORY_CALLSTATE) && ((x) <= EVENT_CATEGORY_MEDIA))


/**
 * Signature for event callback/observer.  Application developers should
 * not block this event callback thread -- doing so will cause deadlocks 
 * and will slow down call processing.  You should re-post these events
 * to your own thread context for handling.  The sipxDuplicateEvent and 
 * sipxFreeDuplicatedEvent methods are available to copy the event callback
 * data (The data is only available for the duration of the callback).  For
 * example, upon receiving the callback, copy the data using 
 * sipxDuplicateEvent(...), post the copied data to your event callback, 
 * process it, and lastly free the event data using 
 * sipxFreeDuplicatedEvent(...).
 *
 * The application developer must look at the SIPX_EVENT_CATEGORY and then 
 * cast the pInfo method to the appropriate structure:
 *
 * <pre>
 * EVENT_CATEGORY_CALLSTATE:   pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo ;
 * EVENT_CATEGORY_LINESTATE:   pLineInfo = (SIPX_LINESTATE_INFO*) pInfo ;
 * EVENT_CATEGORY_INFO_STATUS: pInfoStatus = (SIPX_INFOSTATUS_INFO*) pInfo ;
 * EVENT_CATEGORY_INFO:        pInfoInfo = (SIPX_INFO_INFO*) pInfo ;
 * EVENT_CATEGORY_SUB_STATUS:  pSubInfo = (SIPX_SUBSTATUS_INFO*) pInfo ;
 * EVENT_CATEGORY_NOTIFY:      pNotifyInfo = (SIPX_NOTIFY_INFO*) pInfo ;
 * EVENT_CATEGORY_CONFIG:      pConfigInfo - (SIPX_CONFIG_INFO*) pInfo ;
 * EVENT_CATEGORY_SECURITY:    pSecInfo = (SIPX_SECURITY_INFO*) pInfo ;
 * EVENT_CATEGORY_MEDIA:       pMediaInfo = (SIPX_MEDIA_INFO*) pInfo ;
 * </pre>
 *
 * Please see the SIPX_EVENT_CATEGORY and structure definitions for details.
 *
 * @param category The category of the event (call, line, subscription, 
          notify, etc.).
 * @param pInfo Pointer to the event info structure.  Depending on the event 
 *        type, the application layer needs to cast this parameter to the 
 *        appropriate structure.
 * @param pUserData User data provided when listener was added
 */
typedef bool (SIPX_CALLING_CONVENTION *SIPX_EVENT_CALLBACK_PROC)(SIPX_EVENT_CATEGORY category, 
                                                                 void* pInfo, 
                                                                 void* pUserData);
/**
 * Major call state events identify significant changes in the state of a 
 * call.
 */
typedef enum SIPX_CALLSTATE_EVENT
{
    CALLSTATE_UNKNOWN         = 0,/**< An UNKNOWN event is generated when the state for a call 
                                 is no longer known.  This is generally an error 
                                 condition; see the minor event for specific causes. */
    CALLSTATE_NEWCALL         = 1000, /**< The NEWCALL event indicates that a new call has been 
                                 created automatically by the sipXtapi.  This event is 
                                 most frequently generated in response to an inbound 
                                 call request.  */
	CALLSTATE_DIALTONE        = 2000, /**< The DIALTONE event indicates that a new call has been 
                                 created for the purpose of placing an outbound call.  
                                 The application layer should determine if it needs to 
                                 simulate dial tone for the end user. */
	CALLSTATE_REMOTE_OFFERING = 2500, /**< The REMOTE_OFFERING event indicates that a call setup 
                                 invitation has been sent to the remote party.  The 
                                 invitation may or may not every receive a response.  If
                                 a response is not received in a timely manor, sipXtapi 
                                 will move the call into a disconnected state.  If 
                                 calling another sipXtapi user agent, the reciprocal 
                                 state is OFFER. */
	CALLSTATE_REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup 
                                 invitation has been accepted and the end user is in the
                                 alerting state (ringing).  Depending on the SIP 
                                 configuration, end points, and proxy servers involved, 
                                 this event should only last for 3 minutes.  Afterwards,
                                 the state will automatically move to DISCONNECTED.  If 
                                 calling another sipXtapi user agent, the reciprocate 
                                 state is ALERTING. 
                                 
                                 Pay attention to the cause code for this event.  If
                                 the cause code is "CALLSTATE_CAUSE_EARLY_MEDIA", the 
                                 remote the party is sending early media (e.g. gateway is
                                 producing ringback or audio feedback).  In this case, the
                                 user agent should not produce local ringback. */
	CALLSTATE_CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup 
                                 between the local and remote party.  Network audio should be 
                                 flowing provided and the microphone and speakers should
                                 be engaged. */
    CALLSTATE_BRIDGED         = 5000, /** The BRIDGED state indicates that a call is active,
                                 however, the local microphone/speaker are not engaged.  If
                                 this call is part of a conference, the party will be able
                                 to talk with other BRIDGED conference parties.  Application
                                 developers can still play and record media. */
    CALLSTATE_HELD            = 6000, /** The HELD state indicates that a call is
                                 both locally and remotely held.  No network audio is flowing 
                                 and the local microphone and speaker are not engaged. */
    CALLSTATE_REMOTE_HELD     = 7000, /** The REMOTE_HELD state indicates that the remote 
                                 party is on hold.  Locally, the microphone and speaker are
                                 still engaged, however, no network audio is flowing. */

	CALLSTATE_DISCONNECTED    = 8000, /**< The DISCONNECTED state indicates that a call was 
                                 disconnected or failed to connect.  A call may move 
                                 into the DISCONNECTED states from almost every other 
                                 state.  Please review the DISCONNECTED minor events to
                                 understand the cause. */
	CALLSTATE_OFFERING        = 9000, /**< An OFFERING state indicates that a new call invitation 
                                 has been extended this user agent.  Application 
                                 developers should invoke sipxCallAccept(), 
                                 sipxCallReject() or sipxCallRedirect() in response.  
                                 Not responding will result in an implicit call 
                                 sipXcallReject(). */                                
    CALLSTATE_ALERTING        = 10000, /**< An ALERTING state indicates that an inbound call has 
                                 been accepted and the application layer should alert 
                                 the end user.  The alerting state is limited to 3 
                                 minutes in most configurations; afterwards the call 
                                 will be canceled.  Applications will generally play 
                                 some sort of ringing tone in response to this event. */
    CALLSTATE_DESTROYED       = 11000, /**< The DESTORYED event indicates the underlying resources 
                                 have been removed for a call.  This is the last event 
                                 that the application will receive for any call.  The 
                                 call handle is invalid after this event is received. */
    CALLSTATE_TRANSFER_EVENT   = 12000, /**< The transfer state indicates a state change in a 
                                 transfer attempt.  Please see the CALLSTATE_TRANSFER_EVENT cause 
                                 codes for details on each state transition */
} SIPX_CALLSTATE_EVENT;

 
/**
 * Callstate cuase events identify the reason for a Callstate event or 
 * provide more detail.
 */
typedef enum SIPX_CALLSTATE_CAUSE
{
    CALLSTATE_CAUSE_UNKNOWN,        /**< Unknown cause */
    CALLSTATE_CAUSE_NORMAL,         /**< The stage changed due to normal operation */
	CALLSTATE_CAUSE_TRANSFERRED,	/**< A call is being transferred to this user 
                                         agent from another user agent.*/
	CALLSTATE_CAUSE_TRANSFER,	    /**< A call on this user agent is being transferred 
                                         to another user agent. */                                     
	CALLSTATE_CAUSE_CONFERENCE,     /**< A conference operation caused a stage change */
	CALLSTATE_CAUSE_EARLY_MEDIA,    /**< The remote party is alerting and providing 
                                         ringback audio (early media) */

    CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED, 
                                    /**< The callee rejected a request (e.g. hold) */
	CALLSTATE_CAUSE_BAD_ADDRESS,    /**< The state changed due to a bad address.  This 
                                         can be caused by a malformed URL or network
                                         problems with your DNS server */
	CALLSTATE_CAUSE_BUSY,           /**< The state cahnged because the remote party is
                                         busy */
    CALLSTATE_CAUSE_RESOURCE_LIMIT, /**< Not enough resources are available to complete
                                         the desired operation */
    CALLSTATE_CAUSE_NETWORK,        /**< A network error caused the desired operation to 
                                         fail */
	CALLSTATE_CAUSE_REDIRECTED,     /**< The stage changed due to a redirection of a call. */
	CALLSTATE_CAUSE_NO_RESPONSE,    /**< No response was received from the remote party or 
                                         network node. */
    CALLSTATE_CAUSE_AUTH,           /**< Unable to authenticate due to either bad or 
                                         missing credentials */
    CALLSTATE_CAUSE_TRANSFER_INITIATED,  
                                   /**< A transfer attempt has been initiated.  This event
                                        is sent when a user agent attempts either a blind
                                        or consultative transfer. */
    CALLSTATE_CAUSE_TRANSFER_ACCEPTED,  
                                   /**< A transfer attempt has been accepted by the remote
                                        transferee.  This event indicates that the 
                                        transferee supports transfers (REFER method).  The
                                        event is fired upon a 2xx class response to the SIP
                                        REFER request. */
    CALLSTATE_CAUSE_TRANSFER_TRYING,
                                   /**< The transfer target is attempting the transfer.  
                                        This event is sent when transfer target (or proxy /
                                        B2BUA) receives the call invitation, but before the
                                        the tranfer target accepts is. */
    CALLSTATE_CAUSE_TRANSFER_RINGING,   
                                   /**< The transfer target is ringing.  This event is 
                                        generally only sent during blind transfer.  
                                        Consultative transfer should proceed directly to 
                                        TRANSFER_SUCCESS or TRANSFER_FAILURE. */
    CALLSTATE_CAUSE_TRANSFER_SUCCESS,
                                   /**< The transfer was completed successfully.  The
                                        original call to transfer target will
                                        automatically disconnect.*/
    CALLSTATE_CAUSE_TRANSFER_FAILURE,
                                   /**< The transfer failed.  After a transfer fails,
                                        the application layer is responsible for 
                                        recovering original call to the transferee. 
                                        That call is left on hold. */
    CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED,
                                   /**< Fired if the remote party's user-agent does not
                                        support S/MIME. */
    CALLSTATE_CAUSE_SMIME_FAILURE,
                                   /**< Fired if a local S/MIME operation failed. 
                                        For more information, applications should 
                                        process the SECURITY event. */
    CALLSTATE_CAUSE_SHUTDOWN       /**< The even was fired as part of sipXtapi 
                                        shutdown */
} SIPX_CALLSTATE_CAUSE ;

/**
 * Enumeration of possible linestate Events.
 */
 typedef enum SIPX_LINESTATE_EVENT
{
    LINESTATE_UNKNOWN         = 0,        /**< This is the initial Line event state. */
    LINESTATE_REGISTERING     = 20000,    /**< The REGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message,
                                             but has not yet received a success response from the
                                             registrar server */    
    LINESTATE_REGISTERED      = 21000,    /**< The REGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             registration. */
    LINESTATE_UNREGISTERING   = 22000,    /**< The UNREGISTERING event is fired when sipXtapi
                                             has successfully sent a REGISTER message with an expires=0 parameter,
                                             but has not yet received a success response from the
                                             registrar server */
    LINESTATE_UNREGISTERED    = 23000,    /**< The UNREGISTERED event is fired after sipXtapi has received
                                             a response from the registrar server, indicating a successful
                                             un-registration. */
    LINESTATE_REGISTER_FAILED = 24000,    /**< The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
                                             It is fired in the following cases:  
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_UNREGISTER_FAILED  = 25000, /**< The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION.
                                             It is fired in the following cases:  
                                             The client could not connect to the registrar server.
                                             The registrar server challenged the client for authentication credentials,
                                             and the client failed to supply valid credentials.
                                             The registrar server did not generate a success response (status code == 200)
                                             within a timeout period.  */
    LINESTATE_PROVISIONED      = 26000,   /**< The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is not 
                                             requested (i.e. - sipxLineAdd is called with a bRegister parameter of false. */ 
} SIPX_LINESTATE_EVENT;  


/**
 * Enumeration of possible linestate Event causes.
 */
typedef enum SIPX_LINESTATE_CAUSE
{
    LINESTATE_CAUSE_UNKNOWN                           = 0,                                 /**< No cause specified. */
    LINESTATE_REGISTERING_NORMAL                      = LINESTATE_REGISTERING + 1,         /**< See LINESTATE_REGISTERING
                                                                                                event. */ 
    LINESTATE_REGISTERED_NORMAL                       = LINESTATE_REGISTERED + 1,          /**< See LINESTATE_REGISTERED
                                                                                                event. */
    LINESTATE_UNREGISTERING_NORMAL                    = LINESTATE_UNREGISTERING + 1,       /**< See LINESTATE_UNREGISTERING
                                                                                                event. */
    LINESTATE_UNREGISTERED_NORMAL                     = LINESTATE_UNREGISTERED + 1,        /**< See LINESTATE_UNREGISTERED
                                                                                                event. */
    LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT       = LINESTATE_REGISTER_FAILED + 1,     /**< Failed to register because
                                                                                                of a connectivity problem. */
    LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED          = LINESTATE_REGISTER_FAILED + 2,     /**< Failed to register because
                                                                                                of an authorization / 
                                                                                                authentication failure. */
    LINESTATE_REGISTER_FAILED_TIMEOUT                 = LINESTATE_REGISTER_FAILED + 3,     /**< Failed to register because of
                                                                                                a timeout. */
    LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT     = LINESTATE_UNREGISTER_FAILED + 1,   /**< Failed to unregister because of
                                                                                                a connectivity problem. */
    LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED        = LINESTATE_UNREGISTER_FAILED + 2,   /**< Failed to unregister because of
                                                                                                of an authorization / 
                                                                                                authentication failure. */
    LINESTATE_UNREGISTER_FAILED_TIMEOUT               = LINESTATE_UNREGISTER_FAILED + 3,   /**< Failed to register because of
                                                                                                a timeout. */
    LINESTATE_PROVISIONED_NORMAL                      = LINESTATE_PROVISIONED + 1          /**< See LINESTATE_PROVISIONED
                                                                                                event. */
} SIPX_LINESTATE_CAUSE;




/**
 * Enumeration of possible INFO status events
 */
enum SIPX_INFOSTATUS_EVENT
{
    INFOSTATUS_UNKNOWN       = 0 ,     /**< This is the initial value for an INFOSTATUS event. */
    INFOSTATUS_RESPONSE      = 30000,  /**< This event is fired if a response is received after an
                                            INFO message has been sent */
    INFOSTATUS_NETWORK_ERROR = 31000   /**< This event is fired in case a network error was encountered
                                            while trying to send an INFO event. */
};


/**
 * Enumeration of possible configuration events
 */
enum SIPX_CONFIG_EVENT
{
    CONFIG_UNKNOWN = 0,           /**< Unknown configuration event */
    CONFIG_STUN_SUCCESS  = 40000, /**< A STUN binding has been obtained for signaling purposes.
                                       For a SIPX_CONFIG_EVENT type of CONFIG_STUN_SUCCESS, 
                                       the pData pointer of the info structure will point to a
                                       SIPX_CONTACT_ADDRESS structure. */
    CONFIG_STUN_FAILURE  = 41000, /**< Unable to obtain a STUN binding for signaling purposes. */
} ;


/**
 * Enumeration of possible security events
 */
typedef enum SIPX_SECURITY_EVENT
{
    SECURITY_UNKNOWN       = 0,/**< An UNKNOWN event is generated when the state for a call 
                                 is no longer known.  This is generally an error 
                                 condition; see the minor event for specific causes. */
    SECURITY_ENCRYPT = 1000, /**< The ENCRYPT event indicates that an SMIME encryption has been
                                  attempted.  See the cause code for the encryption outcome,
                                  and the info structure for more information. */
    SECURITY_DECRYPT = 2000, /**< The DECRYPT event indicates that an SMIME decryption has been
                                  attempted.  See the cause code for the encryption outcome,
                                  and the info structure for more information. */
    SECURITY_TLS              = 4000, /**< TLS related security event. */
} SIPX_SECURITY_EVENT;

/**
 * Enumeration of possible security causes
 */
typedef enum SIPX_SECURITY_CAUSE
{
    SECURITY_CAUSE_UNKNOWN = 0,                      /**< An UNKNOWN cause code is generated when the state
                                                          for the security operation 
                                                          is no longer known.  This is generally an error 
                                                          condition; see the info structure for details. */
    SECURITY_CAUSE_NORMAL,                           /**< Event was fired as part of the normal encryption / decryption process. */
    SECURITY_CAUSE_ENCRYPT_SUCCESS,                  /**< An S/MIME encryption succeeded. */
    SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT,         /**< An S/MIME encryption failed because the
                                                          security library could not start. */
    SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY,   /**< An S/MIME encryption failed because of a bad certificate / public key. */
    SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME encryption failed because of an invalid parameter. */
    SECURITY_CAUSE_DECRYPT_SUCCESS,                  /**< An S/MIME decryption succeeded. */ 
    SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT,          /**< An S/MIME decryption failed due to a failure to initialize the certificate database. */
    SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD,  /**< An S/MIME decryption failed due to an invalid certificate database password. */
    SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME decryption failed due to an invalid parameter. */
    SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE,            /**< An S/MIME decryption operation aborted due to a bad signature. */
    SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE,        /**< An S/MIME decryption operation aborted due to a missing signature. */
    SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED,       /**< An S/MIME decryption operation aborted because the signature was rejected. */
    SECURITY_CAUSE_TLS_SERVER_CERTIFICATE,           /**< A TLS server certificate is being presented to the application for possible rejection. 
                                                          The application must respond to this message.
                                                          If the application returns false, the certificate is rejected and the call will not
                                                          complete.  If the application returns true, the certificate is accepted. */
    SECURITY_CAUSE_TLS_BAD_PASSWORD,                /**< A TLS operation failed due to a bad password. */
    SECURITY_CAUSE_TLS_LIBRARY_FAILURE,             /**< A TLS operation failed. */
    SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE,         /**< The remote host is not reachable. */
    SECURITY_CAUSE_TLS_CONNECTION_FAILURE,          /**< A TLS connection to the remote party failed. */
    SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE,           /**< A failure occured during the TLS handshake. */
    SECURITY_CAUSE_SIGNATURE_NOTIFY,                /**< The SIGNATURE_NOTIFY event is fired when the user-agent
                                                         receives a SIP message with signed SMIME as its content.
                                                         The signer's certificate will be located in the info structure
                                                         associated with this event.  The application can choose to accept
                                                         the signature, by returning 'true' in response to this message
                                                         or can choose to reject the signature
                                                         by returning 'false' in response to this message. */
    SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED         /** < The application has rejected the server's TLS certificate. */
} SIPX_SECURITY_CAUSE;


/**
 * Enumeration of possible media events
 */
typedef enum
{
    MEDIA_UNKNOWN     = 0,      /**< Unknown or undefined media event, this is
                                     generally the sign of an internal error in 
                                     sipXtapi */
    MEDIA_LOCAL_START = 50000,  /**< Local media (audio or video) is being 
                                     sent to the remote party */
    MEDIA_LOCAL_STOP,           /**< Local media (audio or video) is no longer
                                     being sent to the remote party.  This may
                                     be caused by a local/remote hold 
                                     operation, call tear down, or error.  See
                                     the SIPX_MEDIA_CAUSE enumeration for more
                                     information. */
    MEDIA_REMOTE_START,         /**< Remote media (audio or video) is ready to
                                     be received.  If no audio/video is 
                                     received for longer then the idle period,
                                     a MEDIA_REMOTE_SILENT event will be fired.
                                     See sipxConfigSetConnectionIdleTimeout. */
    MEDIA_REMOTE_STOP,          /**< Remote media (audio or video) has been 
                                     stopped due to a hold or call tear down.*/
    MEDIA_REMOTE_SILENT,        /**< Remote media has not been received for 
                                     some configured period.  This generally 
                                     indicates a network problem and/or a
                                     problem with the remote party.  See 
                                     sipxConfigSetConnectionIdleTimeout for
                                     more information. */
    MEDIA_PLAYFILE_START,       /**< A file is being played to local and/or 
                                     remote parties.  This event will be
                                     followed by a MEDIA_PLAYFILE_STOP when
                                     the file is manually stopped or 
                                     finished playing. */
    MEDIA_PLAYFILE_STOP,        /**< A file has completed playing or was
                                     aborted.*/
    MEDIA_PLAYBUFFER_START,     /**< A buffer is being played to local and/or 
                                     remote parties.  This event will be
                                     followed by a MEDIA_PLAYBUFFER_STOP when
                                     the file is manually stopped or 
                                     finished playing. */
    MEDIA_PLAYBUFFER_STOP,      /**< A buffer has completed playing or was
                                     aborted.*/
    MEDIA_REMOTE_DTMF,          /**< Not yet implemented */
    MEDIA_DEVICE_FAILURE,       /**< Fired if the media device is not present or
                                     already in use. */

} SIPX_MEDIA_EVENT ;


/**
 * Enumeration of possible media event causes.
 */
typedef enum
{
    MEDIA_CAUSE_NORMAL,             /**< Normal cause; the call was likely torn down.*/
    MEDIA_CAUSE_HOLD,               /**< Media state changed due to a local or remote
                                         hold operation */
    MEDIA_CAUSE_UNHOLD,             /**< Media state changed due to a local or remote
                                         unhold operation */
    MEDIA_CAUSE_FAILED,             /**< Media state changed due to an error condition. */
    MEDIA_CAUSE_DEVICE_UNAVAILABLE, /**< Media state changed due to an error condition,
                                        (device was removed, already in use, etc). */
    MEDIA_CAUSE_INCOMPATIBLE        /**< Incompatible destination -- We were unable
                                         to negotiate a codec */

} SIPX_MEDIA_CAUSE ;

/**
 * Enumeration of possible media event types.  Today, MEDIA_TYPE_AUDIO and
 * MEDIA_TYPE_VIDEO are supported.
 */
typedef enum
{
    MEDIA_TYPE_AUDIO,   /**< Audio media event type */
    MEDIA_TYPE_VIDEO,   /**< Video media event type */

} SIPX_MEDIA_TYPE ;


/**
 * Media event information structure.  This information is passed as part of 
 * the sipXtapi callback mechanism.  Based on the SIPX_EVENT_CATEGORY, the 
 * application developer should cast the pInfo member of your callback to the
 * appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct
{
    size_t              nSize ;     /**< Size of the structure. */
    SIPX_MEDIA_EVENT    event ;     /**< Media event identifier.  See SIPX_MEDIA_EVENT 
                                         for more information. */
    SIPX_MEDIA_CAUSE    cause ;     /**< Media cause identifier.  See SIPX_MEDIA_CAUSE
                                         for more information. */
    SIPX_MEDIA_TYPE     mediaType ; /**< Media type: Either MEDIA_TYPE_AUDIO or 
                                         MEDIA_TYPE_VIDEO. */
    SIPX_CALL           hCall ;     /**< Associate call (or SIPX_CALL_NULL if 
                                         not associated with a call). */
    SIPX_CODEC_INFO     codec ;     /**< Negotiated codec; only supplied on 
                                         MEDIA_LOCAL_START and MEDIA_REMOTE_START
                                         events. */
    int                 idleTime;   /**< Idle time (ms) for SILENT events; only 
                                         supplied on MEDIA_REMOTE_SILENT 
                                         events. */
} SIPX_MEDIA_INFO ;


/**
 * Callstate event information structure.   This information is passed as part of 
 * the sipXtapi callback mechanism.  Based on the SIPX_EVENT_CATEGORY, the 
 * application developer should cast the pInfo member of your callback to the
 * appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct 
{
    // TODO: Add a bitmask that identified which structure items are valid.  For 
    //       example, codec and hAssociatedCall are only valid for certain event
    //       sequences.

    size_t    nSize;                /**< The size of this structure. */
    SIPX_CALL hCall;                /**< Call handle associated with the callstate event. */
    SIPX_LINE hLine;                /**< Line handle associated with the callstate event. */
    SIPX_CALLSTATE_EVENT event;     /**< Callstate event enum code.
                                         Identifies the callstate event. */
    SIPX_CALLSTATE_CAUSE cause;     /**< Callstate cause enum code. 
                                         Identifies the cause of the callstate event. */
    SIPX_CALL hAssociatedCall ;     /**< Call associated with this event.  For example, when
                                         a new call is created as part of a consultative 
                                         transfer, this handle contains the handle of the 
                                         original call. */
} SIPX_CALLSTATE_INFO; 


/**
 * Linestate event information structure.   This information is passed as part of 
 * the sipXtapi callback mechanism.  Based on the SIPX_EVENT_CATEGORY, the 
 * application developer should cast the pInfo member of your callback to the
 * appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct                      
{
    size_t    nSize ;               /**< The size of this structure. */
    SIPX_LINE hLine;                /**< Line handle associated with the linestate event. */ 
    SIPX_LINESTATE_EVENT event ;    /**< Callstate event enum code.
                                         Identifies the linestate event. */
    SIPX_LINESTATE_CAUSE cause ;    /**< Callstate cause enum code. 
                                         Identifies the cause of the linestate event. */
} SIPX_LINESTATE_INFO ;


/**
 *  Major classifications of response statuses for a SIP message.
 */
typedef enum 
{
    SIPX_MESSAGE_OK,                  /**< The message was successfully processed (200) */ 
    SIPX_MESSAGE_FAILURE,             /**< The server received the message, but could or would
                                           not process it. */
    SIPX_MESSAGE_SERVER_FAILURE,      /**< The server encountered an error while trying to process
                                           the message. */
    SIPX_MESSAGE_GLOBAL_FAILURE,      /**< Fatal error encountered. */
} SIPX_MESSAGE_STATUS ;

/**
 * An INFOSTATUS event informs that application layer of the status
 * of an outbound INFO requests.   This information is passed as part of 
 * the sipXtapi callback mechanism.  Based on the SIPX_EVENT_CATEGORY, the 
 * application developer should cast the pInfo member of your callback to the
 * appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct
{
    size_t              nSize ;             /**< the size of this structure in bytes */
    SIPX_INFO           hInfo ;             /**< the handle used to make the outbound info request. */ 
    SIPX_MESSAGE_STATUS status ;            /**< Emumerated status for this
                                                 request acknowledgement. */
    int                 responseCode ;      /**< Numerical status code for this
                                                 request acknowledgement. */
    const char*         szResponseText ;    /**< The text of the request acknowledgement. */
    SIPX_INFOSTATUS_EVENT event;            /**< Event code for this INFO STATUS message */
} SIPX_INFOSTATUS_INFO ;


/**
 * An INFO event signals the application layer that an INFO message
 * was sent to this user agent.  If the INFO message was sent to a 
 * call context (session) hCall will desiginate the call session. 
 *
 * This information is passed as part of the sipXtapi callback mechanism.  
 * Based on the SIPX_EVENT_CATEGORY, the application developer should cast the
 * pInfo member of your callback to the appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct
{
    size_t      nSize ;             /**< Size of structure */
    SIPX_CALL   hCall ;             /**< Call handle if available */
    SIPX_LINE   hLine ;             /**< Line handle if available */
    const char* szFromURL ;         /**< the URL of the host that originated
                                         the INFO message */
    const char* szUserAgent;        /**< the User Agent string of the source agent */
    const char* szContentType ;     /**< string indicating the info content type */
    const char* pContent ;          /**< pointer to the INFO message content */
    size_t      nContentLength ;    /**< length of the INFO message content */
  
} SIPX_INFO_INFO ;

/**
 * Enumeration of the possible subscription states visible to the client.
 */
typedef enum 
{
    SIPX_SUBSCRIPTION_PENDING,      /**< THe subscription is being set up, but not yet active. */
    SIPX_SUBSCRIPTION_ACTIVE ,      /**< The subscription is currently active. */
    SIPX_SUBSCRIPTION_FAILED ,      /**< The subscription is not active due to a failure.*/
    SIPX_SUBSCRIPTION_EXPIRED ,     /**< The subscription's lifetime has expired. */
    // TBD
} SIPX_SUBSCRIPTION_STATE;

/**
 * Enumeration of cause codes for state subscription state changes.
 */
typedef enum
{
    SUBSCRIPTION_CAUSE_UNKNOWN = -1, /**< No cause specified. */
    SUBSCRIPTION_CAUSE_NORMAL     /**< Normal cause for state change. */
} SIPX_SUBSCRIPTION_CAUSE;

/**
 * An SUBSTATUS event informs that application layer of the status
 * of an outbound SUBSCRIPTION requests;
 *
 * This information is passed as part of the sipXtapi callback mechanism.  
 * Based on the SIPX_EVENT_CATEGORY, the application developer should cast the
 * pInfo member of your callback to the appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct 
{
    size_t nSize ;                      /**< The size of this structure in bytes */
    SIPX_SUB hSub ;                     /**< A handle to the subscription to which
                                             this state change occurred. */
    SIPX_SUBSCRIPTION_STATE state ;     /**< Enum state value indicating the current
                                             state of the subscription. */
    SIPX_SUBSCRIPTION_CAUSE cause;      /**< Enum cause for the state change in this
                                             event. */
    const char* szSubServerUserAgent;   /**< The User Agent header field value from
                                             the SIP SUBSCRIBE response (may be NULL) */
    
} SIPX_SUBSTATUS_INFO ;


/**
 * A NOTIFY_INFO event signifies that a NOTIFY message was received for
 * an active subscription.
 *
 * This information is passed as part of the sipXtapi callback mechanism.  
 * Based on the SIPX_EVENT_CATEGORY, the application developer should cast the
 * pInfo member of your callback to the appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */ 
typedef struct 
{
    size_t      nSize ;             /**< The size of this structure in bytes */
    SIPX_SUB    hSub ;              /**< A handle to the subscrption which
                                         caused this NOTIFY event to be received. */
    const char* szNotiferUserAgent; /**< The User-Agent header field value from
                                         the SIP NOTIFY response (may be NULL) */
    const char* szContentType ;     /**< String indicating the info content type */     
    const void* pContent ;          /**< Pointer to the NOTIFY message content */
    size_t      nContentLength ;    /**< Length of the NOTIFY message content in bytes */
} SIPX_NOTIFY_INFO ;


/**
 * SIPX_CONFIG_INFO events signifies that a change in configuration was 
 * observed.
 *
 * NOTE: This structure is subject to change. 
 *
 * This information is passed as part of the sipXtapi callback mechanism.  
 * Based on the SIPX_EVENT_CATEGORY, the application developer should cast the
 * pInfo member of your callback to the appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */ 
typedef struct
{
    size_t            nSize ;   /**< The size of this structure in bytes */
    SIPX_CONFIG_EVENT event ;   /**< Event code -- see SIPX_CONFIG_EVENT for 
                                     details. */
    void*             pData;    /**< Pointer to event data -- SEE SIPX_CONFIG_EVENT
                                     for details. */
} SIPX_CONFIG_INFO ;
					      

/**
 * An SIPX_SECURITY_INFO event informs that application layer of the status
 * of a security operation.
 *
 * This information is passed as part of the sipXtapi callback mechanism.  
 * Based on the SIPX_EVENT_CATEGORY, the application developer should cast the
 * pInfo member of your callback to the appropriate structure.  
 *
 * @see SIPX_EVENT_CALLBACK_PROC
 * @see SIPX_EVENT_CATEGORY
 */
typedef struct
{
    size_t              nSize ;             /**< the size of this structure in bytes */
    char*               szSRTPkey;          /**< the negotiated SRTP key, if any. */
    void*               pCertificate;       /**< pointer to the certificate blob that was
                                                 used to encrypt and/or sign. */
    int                 nCertificateSize;   /**< size of the certificate blob */
    SIPX_SECURITY_EVENT event;              /**< Event code for this SECURITY_INFO message */
    SIPX_SECURITY_CAUSE cause;              /**< Cause code for this SECURITY_INFO message*/
    char*               szSubjAltName;      /**< Populated for SECURITY_CAUSE_SIGNATURE_NOTIFY.*/
    char*               callId;             /**< Points to a call-id string associated with the event.
                                                 Can be NULL. */
    SIPX_CALL           hCall;              /**< A call handle associated with the event.  Can be 0, 
                                                 to signify that the event is not associated with a call. */
    char*               remoteAddress;      /**< A remote address associated with the event.  Can be NULL. */
} SIPX_SECURITY_INFO ;


/* ============================ FUNCTIONS ================================= */

/**
 * Duplicate the event information for a sipXtapi event.  This method is only
 * needed if you wish to post the event information to another thread context.
 * Once the event has been handle, you must call sipxFreeDuplicatedEvent on
 * the copy to avoid memory leaks.
 *
 * @param category Category type supplied by the sipXtapi event callback.
 * @param pEventSource Source of event data supplied by the sipXtapi event 
 *        callback.
 * @param pEventCopy New copy of the event data.  This data must be freed
 *        by calling sipxFreeDuplicatedEvent.
 */
SIPXTAPI_API SIPX_RESULT sipxDuplicateEvent(SIPX_EVENT_CATEGORY category, 
                                            const void*         pEventSource, 
                                            void**              pEventCopy) ;


/**
 * Frees up memory allocated as part of sipxDuplicateEvent. Do not call this
 * API on pointers received as part of the sipXtapi call back.
 *
 * @param category Category type supplied by the sipXtapi event callback.
 * @param pEventSource Copy of event data supplied by sipxDuplicateEvent.
 */
SIPXTAPI_API SIPX_RESULT sipxFreeDuplicatedEvent(SIPX_EVENT_CATEGORY category, 
                                                 void*               pEventCopy) ;

/**
 * Add a callback/observer for the purpose of receiving sipXtapi events
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function to receive sipx events
 * @param pUserData user data passed along with event data
 */
SIPXTAPI_API SIPX_RESULT sipxEventListenerAdd(const SIPX_INST hInst,
                                             SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                             void *pUserData);
                                             
/**
 * Remove a sipXtapi event callback/observer.  Supply the same
 * pCallbackProc and pUserData values as sipxEventListenerAdd.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param pCallbackProc Function used to receive sipx events
 * @param pUserData user data specified as part of sipxListenerAdd
 */
SIPXTAPI_API SIPX_RESULT sipxEventListenerRemove(const SIPX_INST hInst, 
                                                 SIPX_EVENT_CALLBACK_PROC pCallbackProc, 
                                                 void* pUserData) ;

/* ============================ FUNCTIONS ================================= */

/**
 * Create a printable string version of the designated call state event ids.
 * This is generally used for debugging.
 *
 * @param event sipxtapi event code
 * @param cause sipxtapi cause event code
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxCallEventToString(SIPX_CALLSTATE_EVENT event,
                                         SIPX_CALLSTATE_CAUSE cause,
                                         char*  szBuffer,
                                         size_t nBuffer) ;


/**
 * Create a printable string version of the designated event.
 * This is generally used for debugging.
 *
 * @param category Event category code
 * @param pEvent Pointer to the Event.
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxEventToString(const SIPX_EVENT_CATEGORY category,
                                     const void* pEvent,
                                     char*  szBuffer,
                                     size_t nBuffer);

/**
 * Create a printable string version of the designated line event ids.
 * This is generally used for debugging.
 *
 * @deprecated Use sipxEventToString instead.
 * @param event major event type id
 * @param cause event type id
 * @param szBuffer buffer to store event string
 * @param nBuffer length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxLineEventToString(SIPX_LINESTATE_EVENT event,
                                         SIPX_LINESTATE_CAUSE cause,
                                         char*  szBuffer,
                                         size_t nBuffer);
 

/**
 * Create a printable string version of the designated config event.
 * This is generally used for debugging.
 *
 * @param event Configuration event id
 * @param szBuffer Buffer to store event string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxConfigEventToString(SIPX_CONFIG_EVENT event, 
                                           char* szBuffer, 
                                           size_t nBuffer) ;

/** 
 * Create a printable string version of the designated subscription status state. 
 * This is generally used for debugging. 
 * 
 * @param state Subscription state id 
 * @param szBuffer Buffer to store event string 
 * @param nBuffer Length of string buffer szBuffer 
 */ 
SIPXTAPI_API char* sipxSubStatusStateToString(SIPX_SUBSCRIPTION_STATE state, 
                                            char* szBuffer, 
                                            size_t nBuffer) ; 
                                            
/** 
 * Create a printable string version of the designated subscription status cause. 
 * This is generally used for debugging. 
 * 
 * @param cause Subscription cause id 
 * @param szBuffer Buffer to store event string 
 * @param nBuffer Length of string buffer szBuffer 
 */ 
SIPXTAPI_API char* sipxSubStatusCauseToString(SIPX_SUBSCRIPTION_CAUSE cause, 
                                            char* szBuffer, 
                                            size_t nBuffer) ; 

/**
 * Create a printable string version of the designated security event.
 * This is generally used for debugging.
 *
 * @param event Security event id
 * @param szBuffer Buffer to store event string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxSecurityEventToString(SIPX_SECURITY_EVENT event, 
                                           char* szBuffer, 
                                           size_t nBuffer);

/**
 * Create a printable string version of the designated security cause.
 * This is generally used for debugging.
 *
 * @param cause Security cause id
 * @param szBuffer Buffer to store cause string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxSecurityCauseToString(SIPX_SECURITY_CAUSE cause, 
                                           char* szBuffer, 
                                           size_t nBuffer);
                                     

/**
 * Create a printable string version of the designated media event.
 * This is generally used for debugging.
 *
 * @param event Media event id
 * @param szBuffer Buffer to store event string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxMediaEventToString(SIPX_MEDIA_EVENT event,
                                          char* szBuffer,
                                          size_t nBuffer);
/**
 * Create a printable string version of the designated media cause.
 * This is generally used for debugging.
 *
 * @param cause Media cause id
 * @param szBuffer Buffer to store cause string
 * @param nBuffer Length of string buffer szBuffer
 */
SIPXTAPI_API char* sipxMediaCauseToString(SIPX_MEDIA_CAUSE cause,
                                          char* szBuffer,
                                          size_t nBuffer);
                                                                                    
#endif /* ifndef _sipXtapiEvents_h_ */
