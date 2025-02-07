//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Daniel Petrie (dpetrie AT SIPez DOTcom)


#ifndef _SipPimClient_h_
#define _SipPimClient_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <utl/UtlString.h>
#include <net/Url.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;

// Author: Daniel Petrie
//         dgpetrie AT yahoo DOT com
//
//! Client for SIP Presence and Instant Messaging
/*! This client provides interfaces for publishing
 * the current presence state of an entity or AOR.
 * It also provides a basic client for sending
 * pager style instant messages.  That is non-session
 * based messages using the MESSAGE request.  Note
 * that a user interface can provide a session base
 * end user experience using a non-session based
 * messages.
 * 
 *
 * \par Presence State
 * The setPresenceState method can be sued to set the
 * current presence state for the presence AOR.  Setting
 * the state to a different value implies that a PUBLISH
 * request is sent to update the presence state.
 *
 * \par Instant Message
 * A sessionless instant message is sent using the sendPagerMessage
 * method.
 *
 */

class SipPimClient : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    typedef enum 
    {
        invalid = -1,
        closed,
        open
    } SipxRpidStates;

    typedef void (*MessageCallback)(void* userData, const UtlString& fromAddress, const char* textMessage, int textLength, const char* subject, const SipMessage& messageRequest);
/* ============================ CREATORS ================================== */

    //! Constructor
    /*! Construct and bind AOR and user agent for Presence and IM client.
     * \param userAgent - user agent to send requests and receive reponses
     * \param presentityAor - the entity whose presence state is to be
     * published.  This is also the From nameAddr used for instant
     * messages sent.
     */
    SipPimClient(SipUserAgent& userAgent, Url& presentityAor);

    //! Destructor
    virtual
    ~SipPimClient();

/* ============================ MANIPULATORS ============================== */

    //! Set the Private key and Certificate for the local side
    /*! 
     *   \param localPkcs12DerKeyCert DER format PKCS12 container for 
     *          private key and certificate for the local side.
     *   \param localPkcs12DerKeyCertLength the length of localPkcs12DerKeyCert
     *          in bytes
     *   \param sPkcs12Password password (symmetric key) to decrypt the 
     *          PKCS12 container/wrapper.
     */
    void setLocalKeyCert(const char* localPkcs12DerKeyCert,
                         int localPkcs12DerKeyCertLength,
                         const char* sPkcs12Password);

    //! Set the certificate containing the public key for the remote side
    /*! Setting the remote certificate enable encryption of IM messages
     *  to the other side.
     *  \param derFormatCertificate - DER format certificate
     *  \param derFormatCertificateLength - length of the certificate
     */
    void setRemoteCertificate(const char* derFormatCertificate,
                              int derFormatCertificateLength);

    //! Send a pager style instant message to the given destination
    /*! Send a non-session based instant message using the
     * MESSAGE method.
     *
     * \param destinationAor - destination to send the IM to.
     * \param messageText - text of the message to send in the IM
     * \param responseCode - the response code from the request to sent the text
     * \param responseCodeText - the response code text from the top header line
     */
    UtlBoolean sendPagerMessage(Url& destinationAor, 
                                const char* messageText, const char* subject,
                                int& responseCode,
                                UtlString& responseCodeText);

    //! Set callback function for incoming pager mode text messages 
    /*! Set the function to be called if there are incoming MESSAGE
     * requests with text bodies.  Note: the SipPimClient task MUST
     * be started up for the callback to get invoked.
     * \param textHandler - callback function with the following arguments:
     * \param fromAddress - the from field from the request
     * \param textMessage - the text message from the request body
     * \param messageRequest - the MESSAGE request
     */
    void setIncomingImTextHandler(MessageCallback callback, void* userData);

    //! Update the presence state of the presentity indicate 
    /*! Send a PUBLISH request to update the RPID document with the
     * given presence state.  If the current state is the same
     * the presence state document is not updated.
     *
     * \param newState - the new RPID presence state to be published.
     */
    UtlBoolean updatePresenceState(SipxRpidStates newState);

    //! Handler for incoming SIP messages
    /*! This consumes SIP messages and dispatched them to the
     * callbacks.
     */
    UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    //! Call Id generator for SipPimClient
    void getNextCallId(UtlString& callId);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


    //! Disabled copy constructor
    SipPimClient(const SipPimClient& rSipPimClient);

    //! Disabled assignment operator
    SipPimClient& operator=(const SipPimClient& rhs);

    int mCallIdIndex;
    Url mPresentityAor;
    UtlString mFromField;
    SipUserAgent* mpUserAgent;

    MessageCallback mpTextHandlerFunction;
    void* mpTextHandlerUserData;

    UtlString mPkcs12KeyCertContainer;
    UtlString mPkcs12Password;
    UtlString mDerRemoteCertificate;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPimClient_h_
