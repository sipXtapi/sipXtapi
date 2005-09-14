// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////


// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipPimClient.h>
#include <net/HttpBody.h>
#include <net/SmimeBody.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/NetMd5Codec.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipPimClient::SipPimClient(SipUserAgent& userAgent, 
                           Url& presentityAor)
{
    mCallIdIndex = 0;

    presentityAor.toString(mFromField);
    mPresentityAor = presentityAor;
    mpUserAgent = &userAgent;

    // Register to get incoming MESSAGE requests
    OsMsgQ* myQueue = getMessageQueue();
    userAgent.addMessageObserver(*myQueue, 
                                SIP_MESSAGE_METHOD, 
                                TRUE, // requests
                                FALSE, // responces
                                TRUE, // incoming
                                FALSE); // outgoing

}

// Copy constructor
SipPimClient::SipPimClient(const SipPimClient& rSipPimClient)
{
}

// Destructor
SipPimClient::~SipPimClient()
{
}

/* ============================ MANIPULATORS ============================== */

void SipPimClient::setLocalKeyCert(const char* localPkcs12DerKeyCert,
                                   int localPkcs12DerKeyCertLength,
                                   const char* sPkcs12Password)
{
    mPkcs12KeyCertContainer.remove(0);
    if(localPkcs12DerKeyCert && *localPkcs12DerKeyCert &&
        localPkcs12DerKeyCertLength > 0)
    {
        mPkcs12KeyCertContainer.append(localPkcs12DerKeyCert, localPkcs12DerKeyCertLength);
    }

    if(sPkcs12Password && *sPkcs12Password)
    {
        mPkcs12Password = sPkcs12Password;
    }
    else
    {
        sPkcs12Password = "";
    }
}


// Assignment operator
SipPimClient&
SipPimClient::operator=(const SipPimClient& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

//! Send a pager style instant message to the given destination
UtlBoolean SipPimClient::sendPagerMessage(Url& destinationAor, 
                                          const char* messageText,
                                          int& responseCode,
                                          UtlString& responseCodeText)
{
    UtlBoolean returnCode = FALSE;
    responseCode = -1;
    responseCodeText.remove(0);

    if(messageText && *messageText)
    {
        // Construct the text body
        HttpBody* textBody = new HttpBody(messageText,
                                          strlen(messageText),
                                          CONTENT_TYPE_TEXT_PLAIN);

        // Construct the MESSAGE request
        UtlString toAddress = destinationAor.toString();
        UtlString requestUri;
        destinationAor.getUri(requestUri);
        UtlString callId;
        getNextCallId(callId);
        SipMessage messageRequest;
        messageRequest.setRequestData(SIP_MESSAGE_METHOD, requestUri,
                         mFromField, toAddress,
                         callId,
                         1, // sequenceNumber
                         NULL); // contactUrl

        // Attache the body
        messageRequest.setBody(textBody);
        messageRequest.setContentType(CONTENT_TYPE_TEXT_PLAIN);

        // Set the queue to which the response will be deposited
        // for this specific request.
        OsMsgQ responseQueue;
        messageRequest.setResponseListenerQueue(&responseQueue);

        // Send the request 
        returnCode = mpUserAgent->send(messageRequest);

        // wait for the response
        OsMsg* qMessage = NULL;
        // For now we will block forever.  Theoretically this should
        // always get a response (e.g. worst case a 408 timed out).
        // If we do not wait forever, we need to be sure to wait the
        // the maximum transaction timeout period so that the qMessage
        // exists when the SipUserAgent queues the response.
        responseQueue.receive(qMessage);


        // If we got a response, get the response status code and text
        if(qMessage)
        {
            int msgType = qMessage->getMsgType();
            int msgSubType = qMessage->getMsgSubType();

            // SIP message
            if(msgType == OsMsg::PHONE_APP &&
               msgSubType == SipMessage::NET_SIP_MESSAGE)
            {
                const SipMessage* messageResponse = 
                    ((SipMessageEvent*)qMessage)->getMessage();

                if(messageResponse && messageResponse->isResponse())
                {
                    responseCode = messageResponse->getResponseStatusCode();
                    messageResponse->getResponseStatusText(&responseCodeText);
                }
            }
        }
    }

    return(returnCode);
}

void SipPimClient::setIncomingImTextHandler(
                           void (*textHandler)(const UtlString& fromAddress,
                                 const char* textMessage,
                                 int textLength,
                                 const SipMessage& messageRequest))
{
    mpTextHandlerFunction = textHandler;
}


//! Update the presence state of the presentity indicate 
UtlBoolean SipPimClient::updatePresenceState(SipxRpidStates newState)
{
    UtlBoolean returnCode = FALSE;
    return(returnCode);
}

UtlBoolean SipPimClient::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a MESSAGE request
        UtlString method;
        if(sipMessage) sipMessage->getRequestMethod(&method);
        method.toUpper();
        UtlBoolean responseSent = FALSE;
        if(sipMessage &&
            method.compareTo(SIP_MESSAGE_METHOD) == 0 &&
            !sipMessage->isResponse())
        {
            const HttpBody* messageBody = sipMessage->getBody();
            UtlString contentType = messageBody->getContentType();


            // We have a text body and a callback handler function
            if(messageBody && 
               mpTextHandlerFunction &&
               contentType.index(CONTENT_TYPE_TEXT_PLAIN, 0, UtlString::ignoreCase) == 0)
            {
                const char* bodyBytes;
                int bodyLength;
                messageBody->getBytes(&bodyBytes, &bodyLength);
                UtlString fromField;
                sipMessage->getFromField(&fromField);

                // Send back a 200 response
                SipMessage response;
                response.setResponseData(sipMessage, SIP_OK_CODE, SIP_OK_TEXT);
                mpUserAgent->send(response);
                responseSent = TRUE;

                // Invoke the call back with the info
                mpTextHandlerFunction(fromField, bodyBytes, bodyLength, 
                    *sipMessage);

            }
            // S/MIME
            else if(messageBody && 
               mpTextHandlerFunction &&
               contentType.index(CONTENT_SMIME_PKCS7, 0, UtlString::ignoreCase) == 0 &&
               mPkcs12KeyCertContainer.length() > 0)
            {
                SmimeBody* smimeBody = (SmimeBody*) messageBody;
                UtlBoolean decryptedOk =
                    smimeBody->decrypt(mPkcs12KeyCertContainer.data(),
                                       mPkcs12KeyCertContainer.length(),
                                       mPkcs12Password);
                const HttpBody* decryptedBody = NULL;
                UtlString decryptedContentType;
                if(decryptedOk)
                {
                    decryptedBody =
                        smimeBody->getDecyptedBody();

                    if(decryptedBody)
                    {
                        decryptedContentType = 
                            decryptedBody->getContentType();
                        if(decryptedContentType.index(CONTENT_TYPE_TEXT_PLAIN, UtlString::ignoreCase) == 0)
                        {
                            const char* bodyBytes;
                            int bodyLength;
                            messageBody->getBytes(&bodyBytes, &bodyLength);
                            UtlString fromField;
                            sipMessage->getFromField(&fromField);

                            // Send back a 200 response
                            SipMessage response;
                            response.setResponseData(sipMessage, SIP_OK_CODE, SIP_OK_TEXT);
                            mpUserAgent->send(response);
                            responseSent = TRUE;


                            // Invoke the call back with the info
                            mpTextHandlerFunction(fromField, bodyBytes, bodyLength, 
                                *sipMessage);
                        }
                    }

                }
                else
                {
                    UtlString localUri = mPresentityAor.toString();

                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "Unable to decrypt S/MIME MESSAGE Remote: %s Local: %s",
                        mFromField.data(),
                        localUri.data());
                }
                
            }

            if(!responseSent)
            {
                // Send an error as we do not accept the content type
                SipMessage badContentResponse;
                badContentResponse.setResponseData(sipMessage, 
                                                    SIP_BAD_MEDIA_CODE, 
                                                    SIP_BAD_MEDIA_TEXT);
                mpUserAgent->send(badContentResponse);
            }

        }
    }
    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SipPimClient::getNextCallId(UtlString& callId)
{
    UtlString callIdSeed;
    mpUserAgent->getContactUri(&callIdSeed);

    char num[20];

    int epochTime = OsDateTime::getSecsSinceEpoch();
    sprintf(num, "%d", epochTime);
    callIdSeed.append(num);

    mCallIdIndex++;
    sprintf(num, "%d", mCallIdIndex);
    callIdSeed.append(num);

    callIdSeed.append(mFromField);

    NetMd5Codec::encode(callIdSeed, callId);
    callId.append("-pimc");
    callId.append(num);


}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
