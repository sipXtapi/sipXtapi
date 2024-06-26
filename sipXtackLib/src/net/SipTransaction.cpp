//
// Copyright (C) 2006-2019 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////



// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEvent.h>
#include <net/SipTransaction.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipMessageEvent.h>
#include <net/NetMd5Codec.h>
#include <net/SipTransactionList.h>
#include <os/OsSocket.h>
#include <tapi/sipXtapiInternal.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SIP_UDP_RESEND_TIMES 7
#define MIN_Q_DELTA_SQUARE 0.0000000001 // Smallest Q difference is 0.00001
#define BRANCH_ID_PREFIX "z9hG4bK"

// STATIC VARIABLE INITIALIZATIONS
int SipTransaction::smTransactionNum = 0;
UtlString SipTransaction::smBranchIdBase;
//#define TEST_PRINT
//#define LOG_FORKING
//#define ROUTE_DEBUG

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTransaction::SipTransaction(SipMessage* request,
                               UtlBoolean isOutgoing,
                               UtlBoolean userAgentTransaction) :
    mRequestMethod(""),
    mpTransport(NULL)
{
   mIsUaTransaction = userAgentTransaction;

   mpRequest = NULL;
   mpLastProvisionalResponse = NULL;
   mpLastFinalResponse = NULL;
   mpAck = NULL;
   mpCancel = NULL;
   mpCancelResponse = NULL;
   mpParentTransaction = NULL;
   mIsCanceled = FALSE;
   mIsRecursing = FALSE;
   mQvalue = 1.0;
   mExpires = -1;
   mIsBusy = FALSE;
   mWaitingList = NULL;
   mDispatchedFinalResponse = FALSE;
   mProvisionalSdp = FALSE;
   mpDnsSrvRecords = NULL;
   mIsDnsSrvChild = FALSE;
   mSendToPort = PORT_NONE;
   mSendToProtocol = OsSocket::UNKNOWN;

#  ifdef ROUTE_DEBUG
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipTransaction::_ new %p msg %p %s %s",
                    this, &request,
                    isOutgoing ? "OUTGOING" : "INCOMING",
                    userAgentTransaction ? "UA" : "SERVER"
                    );
   }
#  endif

   if(request)
   {
       mIsServerTransaction = request->isServerTransaction(isOutgoing);

       request->getCallIdField(&mCallId);

       // Set the hash key
       buildHash(*request, isOutgoing, *this);

       request->getCSeqField(&mCseq, &mRequestMethod);
       if(!request->isResponse())
       {
           request->getRequestUri(&mRequestUri);
           request->getRequestMethod(&mRequestMethod);

           // Do not attach the request here as it will get passed in
           // later for handleOutgoing or handleIncoming

           if(   0 != mRequestMethod.compareTo(SIP_INVITE_METHOD) // not INVITE
              || !request->getExpiresField(&mExpires))            // or no Expires header field
           {
               mExpires = -1;
           }
       }
       else // this is a response
       {
           // Do not attach the response here as it will get passed in
           // later for handleOutgoing or handleIncoming
       }

       request->getToUrl(mToField);
       request->getFromUrl(mFromField);

       if(!mIsServerTransaction) // is this a new client transaction?
       {
           // Yes - create a new branch id
           getNewBranchId(*request, mBranchId);           
       }
       else
       {
          // This is a server transaction, so the branch id is
          // created by the client and passed in the via
           UtlString viaField;
           request->getViaFieldSubField(&viaField, 0);
           SipMessage::getViaTag(viaField.data(), "branch", mBranchId);
       }
   }
   else
   {
       mIsServerTransaction = FALSE;
   }

   mTransactionState = TRANSACTION_LOCALLY_INIITATED;
   mTransactionStartTime = -1;
   touch();
   mTransactionCreateTime = mTimeStamp;
}

// Copy constructor
SipTransaction::SipTransaction(const SipTransaction& rSipTransaction)
{
}

// Destructor
SipTransaction::~SipTransaction()
{
#ifdef TEST_PRINT
    osPrintf("*******************************\n");
    osPrintf("Deleting messages at:\n");
    osPrintf("    %X\n",mpRequest);
    osPrintf("    %X\n",mpLastProvisionalResponse);
    osPrintf("    %X\n",mpLastFinalResponse);
    osPrintf("    %X\n",mpAck);
    osPrintf("    %X\n",mpCancel);
    osPrintf("    %X\n",mpCancelResponse);
    osPrintf("*******************************\n");
#endif

    // Optimization: stop timers before doing anything else
    deleteTimers();

    if(mpRequest) delete mpRequest;
    mpRequest = NULL;

    if(mpLastProvisionalResponse) delete mpLastProvisionalResponse;
    mpLastProvisionalResponse = NULL;

    if(mpLastFinalResponse) delete mpLastFinalResponse;
    mpLastFinalResponse = NULL;

    if(mpAck) delete mpAck;
    mpAck = NULL;

    if(mpCancel) delete mpCancel;
    mpCancel = NULL;

    if(mpCancelResponse) delete mpCancelResponse;
    mpCancelResponse = NULL;

    if(mpDnsSrvRecords)
    {
       delete[] mpDnsSrvRecords;
    }

    if(mWaitingList)
    {
        int numEvents = mWaitingList->entries();

        if(mpParentTransaction)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::~SipTransaction non parent has %d waiting events",
                numEvents);
        }

        if(numEvents > 0)
        {
            // Cannot call signalAllAvailable as it traverses what
            // may be a broken (i.e. partially deleted) tree
            UtlVoidPtr* eventNode = NULL;
            while ((eventNode = (UtlVoidPtr*) mWaitingList->get()))
            {
                if(eventNode)
                {
                    OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
                    if(waitingEvent)
                    {
                        // If it is already signaled, the other side
                        // is no longer waiting for the event, so this
                        // side must delete the event.
                        if(waitingEvent->signal(0) == OS_ALREADY_SIGNALED)
                        {
                            delete waitingEvent;
                            waitingEvent = NULL;
                        }
                    }
                    delete eventNode;
                    eventNode = NULL;
                }
            }

            OsSysLog::add(FAC_SIP, PRI_ERR, 
                          "SipTransaction::~ %d waiting events in list",
                          numEvents);
        }

        delete mWaitingList;
        mWaitingList = NULL;
    }    
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipTransaction&
SipTransaction::operator=(const SipTransaction& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipTransaction::getNewBranchId(SipMessage& request,
                    UtlString& branchId)
{
   branchId.remove(0);
   branchId.append(BRANCH_ID_PREFIX); // Magic cookie must be in front
   branchId.append("-");

   // :TODO: it would be interesting to add some debugging data here
   //        For example, if we could encode what sort of
   //        transaction this is and where it is in the tree (what # child?)
   //        into a short code here so that we could recognize it in traces...

    // build up the unique part of the branch id by hashing
    //  - a value unique to this call
    //  - a value unique to this host
    //  - a monotonically increasing counter
    UtlString branchUniqCode;
    UtlString branchSeed; // input to the hash
    request.getCallIdField(&branchSeed);

    if(smBranchIdBase.isNull())
    {
        // use the ip address as a value unique to this host (better be true)
        OsSocket::getHostIp(&smBranchIdBase);
    }
    branchSeed.append(smBranchIdBase);

    int transNum = (smTransactionNum++);
    char buf[32];
    sprintf(buf, "%d", transNum);
    branchSeed.append(buf);

    NetMd5Codec::encode(branchSeed.data(), branchUniqCode);

#ifdef USE_LONG_CALL_IDS
    // MD5 component of branchId to 16 bytes
    branchUniqCode.remove(16) ;
#else
    // MD5 component of branchId to 12 bytes
    branchUniqCode.remove(12) ;
#endif

    branchId.append(branchUniqCode);
}

enum SipTransaction::messageRelationship
SipTransaction::addResponse(SipMessage*& response,
                            UtlBoolean isOutGoing,  // intravert/extravert
                            enum messageRelationship relationship) // casual/serious
{
    if(relationship == MESSAGE_UNKNOWN)
    {
        relationship = whatRelation(*response, isOutGoing);
    }

    switch(relationship)
    {

    case MESSAGE_REQUEST:
        // I do not know why this should ever occur
        // Typically the transaction will first be created with a request
        if(mpRequest)
        {            
           OsSysLog::add(FAC_SIP, PRI_WARNING,
                         "SipTransaction::addResponse of request to existing transaction, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpRequest = response;
        }

        if(mTransactionState < TRANSACTION_CALLING)
        {
            mTransactionState = TRANSACTION_CALLING;
            OsTime time;
            OsDateTime::getCurTimeSinceBoot(time);
            mTransactionStartTime = time.seconds();
        }
        break;

    case MESSAGE_PROVISIONAL:
        if(mpLastProvisionalResponse)
        {
            delete mpLastProvisionalResponse;
        }
        mpLastProvisionalResponse = response;
        if(mTransactionState < TRANSACTION_PROCEEDING)
            mTransactionState = TRANSACTION_PROCEEDING;

        // If check if there is early media
        // We need this state member as there may be multiple
        // provisional responses and we cannot rely upon the fact
        // that the last one has SDP or not to indicate that there
        // was early media or not.
        if(!mProvisionalSdp)
        {
            if((response->getSdpBody(response->getSecurityAttributes())))
            {
                mProvisionalSdp = TRUE;
            }
        }
        break;

    case MESSAGE_FINAL:
        if(mpLastFinalResponse) 
        {
            delete mpLastFinalResponse;
        }
        mpLastFinalResponse = response;
        if(mTransactionState < TRANSACTION_COMPLETE)
            mTransactionState = TRANSACTION_COMPLETE;
        break;

    case MESSAGE_ACK:
    case MESSAGE_2XX_ACK:
        if(mpAck)
        {            
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse ACK already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpAck = response;
        }
        break;

    case MESSAGE_CANCEL:
        if(mpCancel)
        {            
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse CANCEL already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpCancel = response;
        }
        break;

    case MESSAGE_CANCEL_RESPONSE:
        if(mpCancelResponse)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse CANCEL response already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpCancelResponse = response;
        }
        break;

    case MESSAGE_UNKNOWN:
    case MESSAGE_UNRELATED:
    case MESSAGE_DUPLICATE:
    default:        
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipTransaction::addResponse message with bad relationship: %d",
                      relationship);
        delete response ;
        response = NULL;
        break;
    }

    return(relationship);
}

UtlBoolean SipTransaction::handleOutgoing(SipMessage& outgoingMessage,
                                         SipUserAgent& userAgent,
                                         SipTransactionList& transactionList,
                                         enum messageRelationship relationship,
                                         SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;
    
    UtlBoolean isResponse = outgoingMessage.isResponse();
    //UtlString toAddress;
    //UtlString protocol;
    SipMessage* message = &outgoingMessage;
    UtlBoolean sendSucceeded = FALSE;
    UtlString method;
    int cSeq;
    UtlString seqMethod;
    outgoingMessage.getCSeqField(&cSeq, &seqMethod);

    outgoingMessage.getRequestMethod(&method);

    if(relationship == MESSAGE_UNKNOWN)
        relationship = whatRelation(outgoingMessage, TRUE);

    if(relationship == MESSAGE_DUPLICATE)
    {
        // If this transaction was contructed with this message
        // it will appear as a duplicate
        if(!isResponse &&
            mpRequest &&
            !mIsServerTransaction &&
            mpRequest->getTimesSent() == 0 &&
            mRequestMethod.compareTo(method) == 0)
        {
            message = mpRequest;
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::handleOutgoing send of duplicate message");
        }

    }

    UtlBoolean addressRequiresDnsSrvLookup(FALSE);
    UtlString toAddress;
    int port = PORT_NONE;
    OsSocket::IpProtocolSocketType protocol = OsSocket::UNKNOWN;

    if(isResponse)
    {
        UtlString protocolString;
        message->getResponseSendAddress(toAddress,
                                        port,
                                        protocolString);
#       ifdef ROUTE_DEBUG
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleOutgoing called getResponseSendAddress, returned toAddress = '%s', port = %d, protocolString = '%s'",
                      toAddress.data(), port, protocolString.data());
#       endif
        SipMessage::convertProtocolStringToEnum(protocolString.data(),
                                        protocol);
    }
    else
    {
        // Fix the request so that it is ready to send
        prepareRequestForSend(*message,
                              userAgent,
                              addressRequiresDnsSrvLookup,
                              toAddress,
                              port,
                              protocol);
#       ifdef ROUTE_DEBUG
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleOutgoing called prepareRequestForSend, returned toAddress = '%s', port = %d, protocol = OsSocket::SocketProtocolTypes(%d), addressRequiresDnsSrvLookup = %d",
                      toAddress.data(), port, protocol,
                      addressRequiresDnsSrvLookup);
#       endif

        if(mSendToAddress.isNull())
        {
#       ifdef ROUTE_DEBUG
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::handleOutgoing setting mSendTo* variables");
#       endif
            mSendToAddress = toAddress;
            mSendToPort = port;
            mSendToProtocol = protocol;
        }
    }

    // Do not send out CANCEL requests on DNS SRV parents.
    // They do not actually send request and so should not
    // send CANCELs either.
    if(   !isResponse
       && !mIsDnsSrvChild
       && (method.compareTo(SIP_CANCEL_METHOD) == 0))
    {
        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
            UtlString requestString;
            int len;
            outgoingMessage.getBytes(&requestString, &len);
            UtlString transString;
            toString(transString, TRUE);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleOutgoing "
                          "should not send CANCEL on DNS parent\n%s\n%s",
                requestString.data(),
                transString.data());
        }
    }

    // Request that requires DNS SRV lookup
    else if (   !isResponse
             && addressRequiresDnsSrvLookup
             && method.compareTo(SIP_ACK_METHOD) != 0
             && method.compareTo(SIP_CANCEL_METHOD) != 0
             && !mIsDnsSrvChild
             )
    {
        if(mpRequest != NULL)
        {
            if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString requestString;
                int len;
                outgoingMessage.getBytes(&requestString, &len);
                UtlString transString;
                toString(transString, TRUE);
                OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipTransaction::handleOutgoing mpRequest should be NULL\n%s\n%s",
                    requestString.data(),
                    transString.data());
            }
        }

        if(relationship != MESSAGE_REQUEST)
        {
            if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString relString;
                SipTransaction::getRelationshipString(relationship, relString);
                UtlString requestString;
                int len;
                outgoingMessage.getBytes(&requestString, &len);
                UtlString transString;
                toString(transString, TRUE);
                OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipTransaction::handleOutgoing invalid relationship: %s\n%s\n%s",
                    relString.data(),
                    requestString.data(),
                    transString.data());
            }
        }


        // Make a copy to attach to the transaction
        SipMessage* requestCopy = new SipMessage(outgoingMessage);

        addResponse(requestCopy,
                    TRUE, // outgoing
                    relationship);

        // Look up the DNS SRV records, create the children tranaction
        // and start pursuing the first child
        if (SIPX_TRANSPORT_DATA::isCustomTransport(mpTransport))
        {
            protocol = OsSocket::CUSTOM;
        }

        if (protocol == OsSocket::CUSTOM)
        {
           sendSucceeded = recurseDnsSrvChildren(userAgent, transactionList, mpTransport);
        }
        else
        {
            sendSucceeded = recurseDnsSrvChildren(userAgent, transactionList, NULL);
        }
    }

    else
    {
        // check the message's URI - 
        // if "sips" is the scheme, 
        // use SSL_SOCKET as the protocol
        UtlString toUrl;
        message->getToField(&toUrl);
        Url parsedUri(toUrl);
        Url::Scheme scheme = parsedUri.getScheme();
        if (scheme == Url::SipsUrlScheme)
        {
            protocol = OsSocket::SSL_SOCKET;
        }

        if (SIPX_TRANSPORT_DATA::isCustomTransport(mpTransport))
        {
            protocol = OsSocket::CUSTOM;
        }
        
        // also, take the transport hint from the to-field and/or from-field.  If there is a transport= in it, 
        // and it is not tls, tcp, or udp, use custom and set mpTransport
        UtlString tempFromField;
        UtlString tempToField;

        message->getFromField(&tempFromField);
        message->getToField(&tempToField);

        UtlString transport;
        Url tempToUrl(tempToField);
        Url tempFromUrl(tempFromField);
        
        tempToUrl.getUrlParameter("transport", transport, 0);
        if (transport.length() < 1 ||
            transport.compareTo("udp") == 0 ||
            transport.compareTo("tls") == 0 ||
            transport.compareTo("tcp") == 0)
        {
            tempFromUrl.getUrlParameter("transport", transport, 0);
            if (transport.compareTo("udp") == 0 ||
                transport.compareTo("tls") == 0 ||
                transport.compareTo("tcp") == 0)
            {
                transport = "";
            }                
        }
        
        UtlString localIp;
        int dummy;
        
        userAgent.getLocalAddress(&localIp, &dummy);
        if (!mpTransport && transport.length() > 0)
        {
            mpTransport = (SIPX_TRANSPORT_DATA*)userAgent.lookupExternalTransport(transport, localIp);
            if (mpTransport)
            {
                protocol = OsSocket::CUSTOM;
            }
        }
        
        sendSucceeded = doFirstSend(*message,
                                    relationship,
                                    userAgent,
                                    toAddress,
                                    port,
                                    protocol,
                                    mpTransport);

        touch();
    }

    return(sendSucceeded);
} // end handleOutgoing

void SipTransaction::prepareRequestForSend(SipMessage& request,
                                           SipUserAgent& userAgent,
                                           UtlBoolean& addressRequiresDnsSrvLookup,
                                           UtlString& toAddress,
                                           int& port,
                                           OsSocket::IpProtocolSocketType& toProtocol)
{
    UtlString protocol;

    // Make sure max-forwards is set and it is not
    // greater than the default value
    int maxForwards;
    int defaultMaxForwards = userAgent.getMaxForwards();
    if(!request.getMaxForwards(maxForwards) ||
        maxForwards > defaultMaxForwards)
    {
        request.setMaxForwards(defaultMaxForwards);
    }

    // ACKs for 200 response do NOT reuse the INVITEs routing and URI
    UtlBoolean ackFor2xx = FALSE;
    UtlString method;
    request.getRequestMethod(&method);

    if(method.compareTo(SIP_ACK_METHOD) == 0 &&
        mpLastFinalResponse)
    {
        int responseCode;
        responseCode = mpLastFinalResponse->getResponseStatusCode();
        if(responseCode >= SIP_2XX_CLASS_CODE &&
           responseCode < SIP_3XX_CLASS_CODE)
        {
            ackFor2xx = TRUE;
        }

    }
    // If this is an ACK for a 2xx response so we have to look it up
    // We have already figured out how to route this request.
    if(mIsDnsSrvChild &&
       !mSendToAddress.isNull() &&
       !ackFor2xx)
    {
        toAddress = mSendToAddress;
        port = mSendToPort;
        toProtocol = mSendToProtocol;
        addressRequiresDnsSrvLookup = FALSE;

#      ifdef ROUTE_DEBUG
        {
           UtlString protoString;
           SipMessage::convertProtocolEnumToString(toProtocol,protoString);
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::prepareRequestForSend %p - SRV child ready"
                         "   to %s:%d via '%s'",
                         &request,
                         toAddress.data(), port, protoString.data()
                         );
        }
#      endif                      
    }

    // Look at the request and figure out how to route it.
    else
    {
       // For INVITE, process header parameters in the request uri
       if (0 == method.compareTo(SIP_INVITE_METHOD))
       {
           request.applyTargetUriHeaderParams();
       }

        // Use the proxy only for requests
        userAgent.getProxyServer(0, &toAddress, &port, &protocol);
#       ifdef ROUTE_DEBUG
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend %p got proxy toAddress '%s', port %d, protocol '%s'",
                      &request, toAddress.data(), port, protocol.data());
#       endif

        // See if there is a route
        UtlString routeUri;
        UtlString routeAddress;
        int routePort;
        UtlString routeProtocol;
        request.getRouteUri(0, &routeUri);
        Url routeUrlParser(routeUri);
        UtlString dummyValue;
        UtlBoolean nextHopLooseRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
        UtlString maddr;
        routeUrlParser.getUrlParameter("maddr", maddr);

        UtlString routeHost;
        SipMessage::parseAddressFromUri(routeUri.data(),
            &routeHost, &routePort, &routeProtocol);

        // All of this URL maipulation should be done via
        // the Url (routeUrlParser) object.  However to
        // be safe, we are only using it to
        // get the maddr If the maddr is present use it as the address
        if(!maddr.isNull())
        {
            routeAddress = maddr;
        }
        else
        {
            routeAddress = routeHost;
        }

#       ifdef ROUTE_DEBUG
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend %p getting first route uri: '%s'",
                      &request, routeUri.data());
#       endif

        // If there is no route use the configured outbound proxy
        if(routeAddress.isNull())
        {
            // It is already set in toAddress
        }
        else //if(!routeAddress.isNull())
        {

            toAddress = routeAddress;
            port = routePort;
            protocol = routeProtocol;

            // If this is not a loose route set the URI
            UtlString value;
            if(!nextHopLooseRoutes)
            {
               //Change the URI in the first line to the route Uri

               // so pop the first route uri
                request.removeRouteUri(0, &routeUri);
#               ifdef ROUTE_DEBUG
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::prepareRequestForSend %p"
                              " removing route, no proxy, uri: '%s'",
                              &request, routeUri.data());
#               endif

                // We need to push the URI on the end of the routes
                UtlString uri;
                request.getRequestUri(&uri);
                request.addLastRouteUri(uri.data());


                // Set the URI to the popped route
                UtlString ChangedUri;
                routeUrlParser.getUri(ChangedUri);
                request.changeRequestUri(ChangedUri);
            }
#           ifdef ROUTE_DEBUG
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::prepareRequestForSend %p - using route address"
                          "   to %s:%d via '%s'",
                          &request, toAddress.data(), port, protocol.data()
                          );
#           endif                      
        }

        // No proxy, no route URI, try to use URI from message
        if(toAddress.isNull())
        {
                    UtlString uriString;
                    request.getRequestUri(&uriString);
                    Url requestUri(uriString, TRUE);

                    requestUri.getHostAddress(toAddress);
                    port = requestUri.getHostPort();
                    requestUri.getUrlParameter("transport", protocol);
                    if(requestUri.getUrlParameter("maddr", maddr) &&
                            !maddr.isNull())
                    {
                            toAddress = maddr;
                    }
#         ifdef ROUTE_DEBUG
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipTransaction::prepareRequestForSend %p - "
                        "   using request URI address: %s:%d via '%s'",
                        &request, toAddress.data(), port, protocol.data()
                        );
#         endif                      
        }

        // No proxy, route URI, or message URI, use the To field
        if(toAddress.isNull())
        {
           request.getToAddress(&toAddress, &port, &protocol);
#          ifdef ROUTE_DEBUG
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::prepareRequestForSend %p "
                         "No URI address, using To address"
                         "   to %s:%d via '%s'",
                         &request, toAddress.data(), port, protocol.data()
                         );
#          endif
        }

        UtlString toField;
        request.getToField(&toField);
        Url toUrl(toField);
        UtlString toMaddr;
        toUrl.getUrlParameter("maddr", toMaddr);

        // Trying to have a low impact on the code All of this should just use the Url object
        if(!toMaddr.isNull())
        {
            toAddress = toMaddr;
        }

#ifdef TEST_PRINT
        osPrintf("SipTransaction::prepareRequestForSend UA Sending SIP REQUEST to: \"%s\" port: %d\n", toAddress.data(), port);
#endif

        //SDUA
        UtlString sPort;
        UtlString thisMethod;
        request.getRequestMethod(&thisMethod);

        //check if CANCEL method and has corresponding INVITE
        if(strcmp(thisMethod.data(), SIP_CANCEL_METHOD) == 0)
        {
            //Stick DNS parameters because this cancel is for the same transaction as invite
            //find correcponding INVITE request
            //SipMessage * InviteMsg =  sentMessages.getInviteFor( &message);
            if ( !mIsServerTransaction &&
                 mpRequest &&
                 mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                //copy DNS parameters
                if ( mpRequest->getDNSField(&protocol , &toAddress , &sPort))
                {
                    request.setDNSField( protocol , toAddress , sPort);
                }

            }
        }

        // USE CONTACT OR RECORD ROUTE FIELDS FOR 200 OK responses
        //check if ACK method and if it has contact field set
        //if contact field is set then it is a 200 OK response
        //therefore do not set sticky DNS prameters or DNS look up
        //if DNS field is present request
        if (request.getDNSField(&protocol , &toAddress, &sPort))
        {
            port = atoi(sPort.data());
        }
        else
        {
            addressRequiresDnsSrvLookup = TRUE;
        }

        // If no one specified which protocol
        if(protocol.isNull())
        {
            toProtocol = OsSocket::UNKNOWN;
        }
        else
        {
            SipMessage::convertProtocolStringToEnum(protocol.data(),
                                                    toProtocol);
        }
    }
#   ifdef ROUTE_DEBUG
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::prepareRequestForSend %p prepared SIP REQUEST"
                  "   DNS SRV lookup: %s"
                  "   to %s:%d via '%s'",
                  &request,
                  addressRequiresDnsSrvLookup ? "NEEDED" : "NOT NEEDED",
                  toAddress.data(), port, protocol.data()
                  );
#   endif
}

UtlBoolean SipTransaction::doFirstSend(SipMessage& message,
                                      enum messageRelationship relationship,
                                      SipUserAgent& userAgent,
                                      UtlString& toAddress,
                                      int& port,
                                      OsSocket::IpProtocolSocketType& toProtocol,
                                      SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;
    UtlBoolean sendSucceeded = FALSE;
    UtlBoolean isResponse = message.isResponse();
    UtlString method;
    UtlString seqMethod;
    int responseCode = -1;

    OsSocket::IpProtocolSocketType lastSentProtocol = message.getSendProtocol();
    int resendDuration;
    int resendTime;

#   ifdef ROUTE_DEBUG
    {
       UtlString logRelationship;
       SipTransaction::getRelationshipString(relationship, logRelationship);
       UtlString logProtocol;
       SipMessage::convertProtocolEnumToString(toProtocol, logProtocol);
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipTransaction::doFirstSend %p %s to %s:%d via '%s'",
                     &message, logRelationship.data(), toAddress.data(), port, logProtocol.data()
                     );
    }
#   endif

    if(toProtocol == OsSocket::UNKNOWN)
    {
       if ( lastSentProtocol == OsSocket::UNKNOWN )
       {
          /*
           * :HACK: This is a problem, and a more comprehensive fix is still needed. [XSL-49]
           *
           * We get to here when sending an ACK to 2xx responses, and we shouldn't;
           * those really should be going through the normal routing to determine the
           * protocol using DNS SRV lookups, but there is code elsewhere that prevents
           * that from happening. 
           *
           * Forcing UDP may not always be correct, but it's the best we can do now.
           */
           toProtocol = OsSocket::UDP;
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipTransaction::doFirstSend protocol not explicitly set - using UDP"
                        );
       }
       else
       {
          toProtocol = lastSentProtocol;
       }
    }

    // Responses:
    if(isResponse)
    {
        responseCode = message.getResponseStatusCode();
        int cSeq;
        message.getCSeqField(&cSeq, &seqMethod);

#       ifdef ROUTE_DEBUG
        {
           UtlString protocolStr;
           SipMessage::convertProtocolEnumToString(toProtocol, protocolStr);
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::doFirstSend %p "
                         "Sending RESPONSE to: %s:%d via: %s", 
                         this, toAddress.data(), port, protocolStr.data());
        }
#       endif
        // This is the first send, save the address and port to which it get sent
        message.setSendAddress(toAddress.data(), port);
        message.setFirstSent();
    }

    // Requests:
    else
    {
        // This is the first send, save the address and port to which it get sent
        message.setSendAddress(toAddress.data(), port);
        message.setFirstSent();
        message.getRequestMethod(&method);

        // Add a via to requests, now that we know the protocol
        userAgent.prepareVia(message, mBranchId, toProtocol, toAddress.data(), &port, mpTransport);
    }

    if(toProtocol == OsSocket::TCP)
    {
        lastSentProtocol = OsSocket::TCP;
        resendDuration = 0;
        resendTime = userAgent.getReliableTransportTimeout() * 1000;
    }
#   ifdef SIP_TLS
    else if(toProtocol == OsSocket::SSL_SOCKET)
    {
        lastSentProtocol = OsSocket::SSL_SOCKET;
        resendDuration = 0;
        resendTime = userAgent.getReliableTransportTimeout() * 1000;
    }
#   endif
    else if (OsSocket::CUSTOM == toProtocol)
    {
        lastSentProtocol = OsSocket::CUSTOM;
        if (pTransport->bIsReliable)
        {
            resendDuration = 0;
            resendTime = userAgent.getReliableTransportTimeout() * 1000;
        }
        else
        {
            resendTime = userAgent.getFirstResendTimeout() * 1000;
            resendDuration = userAgent.getFirstResendTimeout();
        }
    }
    else
    {
        if(toProtocol != OsSocket::UDP)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                "SipTransaction::doFirstSend %p unknown protocol: %d using UDP",
                &message, toProtocol);

        }

        resendTime = userAgent.getFirstResendTimeout() * 1000;
        resendDuration = userAgent.getFirstResendTimeout();
        lastSentProtocol = OsSocket::UDP;
    }

    // Set the transport information
    message.setResendDuration(resendDuration);
    message.setSendProtocol(lastSentProtocol);
    message.touchTransportTime();

    SipMessage* transactionMessageCopy = NULL;

    if(relationship == MESSAGE_REQUEST ||
        relationship == MESSAGE_PROVISIONAL ||
        relationship == MESSAGE_FINAL ||
        relationship == MESSAGE_CANCEL ||
        relationship == MESSAGE_CANCEL_RESPONSE ||
        relationship == MESSAGE_ACK ||
        relationship == MESSAGE_2XX_ACK)
    {
        // Make a copy to attach to the transaction
        transactionMessageCopy = new SipMessage(message);

        // Need to add the message to the transaction before it
        // is sent to avoid the race of receiving the response before
        // the request is added to the transaction.
        addResponse(transactionMessageCopy,
                    TRUE, // outgoing
                    relationship);
    }

    if(toProtocol == OsSocket::TCP)
    {
#       ifdef TEST_PRINT
        message.logTimeEvent("SENDING TCP");
#       endif
        sendSucceeded = userAgent.sendTcp(&message,
                                      toAddress.data(),
                                      port);
    }

    else if(toProtocol == OsSocket::SSL_SOCKET)
    {

#           ifdef TEST_PRINT
            message.logTimeEvent("SENDING TLS");
#           endif

            sendSucceeded = userAgent.sendTls(&message,
                                          toAddress.data(),
                                          port);
    }
    else if (toProtocol >= OsSocket::CUSTOM)
    {
        sendSucceeded = userAgent.sendCustom(pTransport, &message, toAddress.data(), port);
    }
    else
    {
#       ifdef TEST_PRINT
        message.logTimeEvent("SENDING UDP");
#       endif

        sendSucceeded = userAgent.sendUdp(&message,
                                          toAddress.data(),
                                          port);
    }
    if(   MESSAGE_REQUEST == relationship
       && !sendSucceeded
       )
    {
        mTransactionState = TRANSACTION_TERMINATED;
    }

#   ifdef TEST_PRINT
    message.dumpTimeLog();
#   endif

#   ifdef TEST_PRINT
    osPrintf("SipTransaction::doFirstSend set Scheduling & resend data\n");
#   endif

    // Increment after the send so the logging messages are
    // acurate

    message.incrementTimesSent();
    if(transactionMessageCopy) transactionMessageCopy->incrementTimesSent();

    if(sendSucceeded)
    {
        // Schedule a timeout for requests and final INVITE error
        // responses (2xx class INVITE responses will be resent
        // by user agents only)
        if(   (   ! isResponse
               && strcmp(method.data(), SIP_ACK_METHOD) != 0
               )
           || (   isResponse
               && (   responseCode >= SIP_3XX_CLASS_CODE
                   || (mIsUaTransaction && responseCode >= SIP_OK_CODE)
                   )
               && strcmp(seqMethod.data(), SIP_INVITE_METHOD) == 0
               )
           )
        {
#           ifdef TEST_PRINT
            osPrintf("SipTransaction::doFirstSend Scheduling UDP %s timeout in %d msec\n",
                method.data(),
                userAgent.getFirstResendTimeout());
#           endif
            // Set the transaction for the timeout message to make
            // it easier to find the transaction when the timer fires.
            message.setTransaction(this);
            if(transactionMessageCopy) transactionMessageCopy->setTransaction(this);

            // Keep separate copy for the timer
            SipMessageEvent* resendEvent =
                new SipMessageEvent(new SipMessage(message),
                                SipMessageEvent::TRANSACTION_RESEND);
#           ifdef TEST_PRINT
            osPrintf("SipTransaction::doFirstSend timer scheduled for: %x\n",
                      resendEvent->getMessage());
#           endif

            // Set an event timer to resend the message
            // queue a message on this OsServerTask
            OsMsgQ* incomingQ = userAgent.getMessageQueue();
            OsTimer* timer = new OsTimer(incomingQ,
                (intptr_t)resendEvent);
            mTimers.append(timer);
#ifdef TEST_PRINT
            osPrintf("SipTransaction::doFirstSend added timer %p to timer list.\n", timer);
#endif
            // Convert from mSeconds to uSeconds
            OsTime timerTime(0,
                             resendTime);
            timer->oneshotAfter(timerTime);

            // If this is a client transaction and we are sending
            // a request, set an expires timer for the transaction
            if(!mIsServerTransaction &&
               !isResponse)
            {
                int expireSeconds = mExpires;
                int maxExpires = userAgent.getDefaultExpiresSeconds();
                // We cancel DNS SRV children after the configured DNS SRV timeout.
                // The timeout is ignored if we receive any response.
                // If this is the only child, do not set a short (DNS SRV) timeout
                if(mIsDnsSrvChild &&
                   mpParentTransaction &&
                   mpParentTransaction->isChildSerial())
                {
                    UtlString method;
                    int dummyNum;
                    message.getCSeqField(&dummyNum, &method);
                    if (strcmp(method.data(), SIP_REGISTER_METHOD) == 0)
                    {
                        expireSeconds = userAgent.getRegisterResponseTimeout();
                    }
                    else
                    {
                        expireSeconds = userAgent.getDnsSrvTimeout();
                    }
                }

                // Normal client transaction
                else if(expireSeconds <= 0)
                {
                    if(mpParentTransaction &&
                        mpParentTransaction->isChildSerial())
                    {
                        expireSeconds = userAgent.getDefaultSerialExpiresSeconds();

                    }

                    else
                    {
                        expireSeconds = maxExpires;
                    }
                }

                // Make sure the expiration is not longer than
                // the maximum length of time we keep a transaction around
                if(expireSeconds > maxExpires)
                {
                    expireSeconds = maxExpires;
                }

                // Keep separate copy for the timer
                SipMessageEvent* expiresEvent =
                    new SipMessageEvent(new SipMessage(message),
                                SipMessageEvent::TRANSACTION_EXPIRATION);

                OsTimer* expiresTimer = new OsTimer(incomingQ,
                        (intptr_t)expiresEvent);
                mTimers.append(expiresTimer);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::doFirstSend added timer %p to timer list.\n", expiresTimer);
#endif

                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::doFirstSend %p setting timeout %d",
                              this, expireSeconds
                              );

                OsTime expiresTime(expireSeconds, 0);
                expiresTimer->oneshotAfter(expiresTime);
            }
        }
    }

    return(sendSucceeded);
} // end doFirstSend

void SipTransaction::handleResendEvent(const SipMessage& outgoingMessage,
                                        SipUserAgent& userAgent,
                                        enum messageRelationship relationship,
                                        SipTransactionList& transactionList,
                                        int& nextTimeout,
                                        SipMessage*& delayedDispatchedMessage,
                                        SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    if(delayedDispatchedMessage)
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleResendEvent %p delayedDispatchedMessage is not NULL", this);
        delayedDispatchedMessage = NULL;
    }

    //osPrintf("SipTransaction::handleResendEvent\n");
    nextTimeout = 0;

    // If this is not a duplicate, then there is something worng
    if(relationship != MESSAGE_DUPLICATE &&
        relationship != MESSAGE_CANCEL)
    {
        UtlString relationString;
        getRelationshipString(relationship, relationString);

        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleResendEvent %p timeout message is not duplicate: %s",
            this, relationString.data());
    }

    // Responses
    if(outgoingMessage.isResponse())
    {
        // The only responses which should have a timeout set
        // are INVITE responses for UA server transactions only

        if(mpLastFinalResponse == NULL)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::handleResendEvent response timeout with no response");
        }

        // If this is a user agent server transaction and
        // the ACK has not been received yet
        // We should only be getting here for error final responses
        // when the transaction is on a proxy (i.e. !mIsUaTransaction)
        if(//mIsUaTransaction && // vs. proxy
            mIsServerTransaction && // vs. client
            mpAck == NULL &&
            mpLastFinalResponse)
        {
            // We have not yet received the ACK

            //int nextTimeout = 0;
            // Use mpLastFinalResponse, not outgoingMessage as
            // mpLastFinalResponse may be a newer final response.
            // outgoingMessage is a snapshot that was taken when the
            // timer was set.
            UtlBoolean sentOk = doResend(*mpLastFinalResponse,
                                        userAgent, nextTimeout, mpTransport);

            if(sentOk)
            {
                // Schedule the next timeout
                // As this is a resend, we should be able to use
                // the same copy of the SIP message for the next timeout
#ifdef TEST_PRINT
                if(outgoingMessage.getSipTransaction() == NULL)
                {
                    UtlString msgString;
                    int msgLen;
                    outgoingMessage.getBytes(&msgString, &msgLen);
                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipTransaction::handleResendEvent reschedule of response resend with NULL transaction",
                        msgString.data());
                    osPrintf("SipTransaction::handleResendEvent reschedule of response resend with NULL transaction\n",
                        msgString.data());
                }
#endif

                // Schedule a timeout for requests which do not
                // receive a response
                SipMessageEvent* resendEvent =
                    new SipMessageEvent(new SipMessage(outgoingMessage),
                                    SipMessageEvent::TRANSACTION_RESEND);

                OsMsgQ* incomingQ = userAgent.getMessageQueue();

                OsTimer* timer = new OsTimer(incomingQ,
                    (intptr_t)resendEvent);

                mTimers.append(timer);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleResendEvent added timer %p to timer list.\n", timer);
#endif

                // Convert from mSeconds to uSeconds
                OsTime lapseTime(0,
                    nextTimeout * 1000);
                timer->oneshotAfter(lapseTime);
            }
            else
            {
                if( MESSAGE_REQUEST == relationship )
                {
                    mTransactionState = TRANSACTION_TERMINATED;
                }

                // Do this outside so that we do not get blocked
                // on locking or delete the transaction out
                // from under ouselves
                // Cleanup the message
                //delete outgoingMessage;
                //outgoingMessage = NULL;
                //userAgent.dispatch(outgoingMessage,
                //                   SipMessageEvent::TRANSPORT_ERROR);
            }

        } // A legal response timeout

        // The ACK was received so we can quit
        // We should only be getting here for error final responses
        // when the transaction is on a proxy (i.e. !mIsUaTransaction)
        else if(// do we care if proxy or UA?? mIsUaTransaction && // vs. proxy
            mIsServerTransaction && // vs. client
            mpAck &&
            mpLastFinalResponse)
        {
            nextTimeout = -1;
        }

    }

    // Requests
    else
    {
        // This should never be the case
        if(outgoingMessage.isFirstSend())
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleResendEvent %p called for first time send of message", this);
        }
        else if(!mIsCanceled &&
                mpLastFinalResponse == NULL &&
                mpLastProvisionalResponse == NULL &&
                mTransactionState == TRANSACTION_CALLING)
        {
            UtlString method;
            outgoingMessage.getRequestMethod(&method);

            // This is a resend retrieve the address and port to send the message to.
#ifdef TEST_PRINT
            UtlString toAddress;
            int port;
            outgoingMessage.getSendAddress(&toAddress, &port);
            osPrintf("SipTransaction::handleResendEvent Resend request %s:%d\n", toAddress.data(), port);
#endif

            SipMessage* resendMessage = NULL;
            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
                OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleResendEvent resend of ACK");
                resendMessage = mpAck;
            }
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
                resendMessage = mpCancel;
            }
            else
            {
                resendMessage = mpRequest;
            }
            //int nextTimeout = 0;
            UtlBoolean sentOk = doResend(*resendMessage,
                                        userAgent, nextTimeout, mpTransport);

            if(sentOk &&
                nextTimeout > 0)
            {
                // Schedule the next timeout
#ifdef TEST_PRINT
                if(outgoingMessage.getSipTransaction() == NULL)
                {
                    UtlString msgString;
                    int msgLen;
                    outgoingMessage.getBytes(&msgString, &msgLen);
                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipTransaction::handleResendEvent reschedule of request resend with NULL transaction",
                        msgString.data());
                    osPrintf("SipTransaction::handleResendEvent reschedule of request resend with NULL transaction\n",
                        msgString.data());
                }
#endif
                // As this is a resend, we should be able to use
                // the same copy of the SIP message for the next timeout

                // Schedule a timeout for requests which do not
                // receive a response
                SipMessageEvent* resendEvent =
                    new SipMessageEvent(new SipMessage(outgoingMessage),
                                    SipMessageEvent::TRANSACTION_RESEND);

                OsMsgQ* incomingQ = userAgent.getMessageQueue();

                OsTimer* timer = new OsTimer(incomingQ,
                    (intptr_t)resendEvent);
                mTimers.append(timer);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleResendEvent added timer %p to timer list.\n", timer);
#endif

                // Convert from mSeconds to uSeconds
                OsTime lapseTime(0,
                    nextTimeout * 1000);
                timer->oneshotAfter(lapseTime);
            }
            else
            {
                // Do this outside so that we do not get blocked
                // on locking or delete the transaction out
                // from under ouselves
                // Cleanup the message
                //delete outgoingMessage;
                //outgoingMessage = NULL;
                //userAgent.dispatch(outgoingMessage,
                //                   SipMessageEvent::TRANSPORT_ERROR);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleResendEvent sentOk: %d nextTimeout: %d\n",
                    sentOk, nextTimeout);
#endif
                if(!sentOk)
                {
                    if ( MESSAGE_REQUEST == relationship )
                    {
                        mTransactionState = TRANSACTION_TERMINATED;
                    }
                    else
                    {
                        mTransactionState = TRANSACTION_COMPLETE;
                    }
                }
                else if ( nextTimeout < 0 )
                {
                    mTransactionState = TRANSACTION_COMPLETE;
#ifdef TEST_PRINT
                    osPrintf("SipTransaction::handleResendEvent failed to send request, TRANSACTION_COMPLETE\n");
#endif
                }
                // else nextTimeout == 0, which should mean the
                // final response was received

            }
        }
        else
        {
            // We are all done, do not reschedule and do not
            // send transport error
            nextTimeout = -1;
            if(mTransactionState == TRANSACTION_CALLING)
                mTransactionState = TRANSACTION_COMPLETE;
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleResendEvent no response, TRANSACTION_COMPLETE");
        }
    }

    if(mpParentTransaction)
    {
        mpParentTransaction->handleChildTimeoutEvent(*this,
                                                     outgoingMessage,
                                                     userAgent,
                                                     relationship,
                                                     transactionList,
                                                     nextTimeout,
                                                     delayedDispatchedMessage,
                                                     mpTransport);
    }

    touch();
} // end handleResendEvent

void SipTransaction::handleExpiresEvent(const SipMessage& outgoingMessage,
                                        SipUserAgent& userAgent,
                                        enum messageRelationship relationship,
                                        SipTransactionList& transactionList,
                                        int& nextTimeout,
                                        SipMessage*& delayedDispatchedMessage,
                                        SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

#ifdef TEST_PRINT
    osPrintf( "SipTransaction::handleExpiresEvent %p\n", this );
#endif

    if(delayedDispatchedMessage)
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleExpiresEvent delayedDispatchedMessage not NULL");

        delayedDispatchedMessage = NULL;
    }

    // Responses
    if(outgoingMessage.isResponse())
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleExpiresEvent %p expires event timed out on SIP response", this);
    }

    // Requests
    else
    {
#ifdef TEST_PRINT
        UtlString transactionTreeString;
        dumpTransactionTree(transactionTreeString, FALSE);
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipTransaction::handleExpiresEvent %p\n%s",
            this, transactionTreeString.data());
#else
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleExpiresEvent %p",
                      this);
#endif

        // Do not cancel a DNS SRV child that received any response.
        // The parent client transaction may be canceled which will
        // recursively cancel the children.  However if this timeout
        // event is for a DNS SRV child we do not cancel if there was
        // any sort of response.
        if(mIsDnsSrvChild &&
            (mpLastProvisionalResponse ||
            mpLastFinalResponse))
        {
            // no-op
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                "SipTransaction::handleExpiresEvent %p ignoring cancel of DNS SRV child", this);
        }

        // Do not cancel an early dialog with early media if this
        // transaction is a child to a serial search/fork.
        // This may be a gateway sending IVR prompts ala
        // American Airlines, so we do not want to cancel in
        // the middle of the user entering DTMF
        else if(!mIsDnsSrvChild &&
           !mIsServerTransaction &&
           mpParentTransaction &&
           mpParentTransaction->isChildSerial() &&
           mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
           isChildEarlyDialogWithMedia())
        {
            // no op
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                "SipTransaction::handleExpiresEvent %p ignoring cancel of early media branch of serial search", this);
        }

        // Do not cancel a completed transaction
        else if((mIsRecursing ||
           mTransactionState == TRANSACTION_CALLING ||
           mTransactionState == TRANSACTION_PROCEEDING ||
           mTransactionState == TRANSACTION_LOCALLY_INIITATED))
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                "SipTransaction::handleExpiresEvent %p canceling expired transaction", this);

            // This transaction has expired cancel it
            cancel(userAgent,
                   transactionList);

        }

#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleExpiresEvent this: %p", this);
        UtlString transactionDump;
        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
            dumpTransactionTree(transactionDump, FALSE);
        }
        else
        {
            transactionDump = "NOT DUMPED";
        }
        // dpetrie: The following is very expensive (and not necessary in this context)
        // transactionList.toString(transactionDump);
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s", transactionDump.data());
#endif

        // Check the parents in the heirarchy to see if there are
        // other branches to pursue
        if(mpParentTransaction)
        {
            mpParentTransaction->handleChildTimeoutEvent(*this,
                                                         outgoingMessage,
                                                         userAgent,
                                                         relationship,
                                                         transactionList,
                                                         nextTimeout,
                                                         delayedDispatchedMessage,
                                                         mpTransport);
        }
        // This is the top most parent and it is a client transaction
        // we need to find the best result
        else if(!mIsServerTransaction)
        {
            handleChildTimeoutEvent(*this,
                                     outgoingMessage,
                                     userAgent,
                                     relationship,
                                     transactionList,
                                     nextTimeout,
                                     delayedDispatchedMessage,
                                     mpTransport);
        }

        touch();
    }
}

UtlBoolean SipTransaction::handleChildIncoming(//SipTransaction& child,
                                     SipMessage& incomingMessage,
                                     SipUserAgent& userAgent,
                                     enum messageRelationship relationship,
                                     SipTransactionList& transactionList,
                                     UtlBoolean childSaysShouldDispatch,
                                     SipMessage*& delayedDispatchedMessage,
                                     SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    UtlBoolean shouldDispatch = childSaysShouldDispatch;

#ifdef TEST_PRINT
    {
        UtlString relateStr;
        getRelationshipString( relationship, relateStr );
        osPrintf("SipTransaction::handleChildIncoming %p relate %s childSays %d\n",
                 this, relateStr.data(), childSaysShouldDispatch);
    }
#endif
    //UtlBoolean isResponse = incomingMessage.isResponse();

    if(   relationship == MESSAGE_FINAL
       || relationship == MESSAGE_PROVISIONAL
       )
    {
        int responseCode = incomingMessage.getResponseStatusCode();

        // If there is a parent pass it up
        if(mpParentTransaction)
        {
            // May want to short cut this and first get
            // the top most parent.  However if the state
            // change is interesting to intermediate (i.e.
            // not top most parent) transactions we need to
            // do it this way (recursively)
            shouldDispatch =
                mpParentTransaction->handleChildIncoming(//child,
                                            incomingMessage,
                                            userAgent,
                                            relationship,
                                            transactionList,
                                            childSaysShouldDispatch,
                                            delayedDispatchedMessage,
                                            mpTransport);
#           ifdef TEST_PRINT
            osPrintf("SipTransaction::handleChildIncoming %p parent says %d\n",
                     this, shouldDispatch);
#           endif
        }
        else // this is the topmost parent transaction
        {
            // We do not dispatch if this is a server transaction
            // as the server transaction is the consumer of the
            // message.  If there is no server transaction as the
            // top most parent, then we assume the consumer is a
            // local application (i.e. message queue).
            if(mIsServerTransaction)
            {
               if (   (responseCode >  SIP_TRYING_CODE) // (responseCode >= SIP_2XX_CLASS_CODE)
                   && (responseCode <  SIP_3XX_CLASS_CODE)
                   )
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipTransaction::handleChildIncoming %p "
                                "topmost parent dispatching %d",
                                this, responseCode );
                  shouldDispatch = TRUE;
               }
               else
               {
#                 ifdef DISPATCH_DEBUG
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipTransaction::handleChildIncoming %p "
                                "topmost parent NOT dispatching %d.",
                                this, responseCode );
#                 endif
                  shouldDispatch = FALSE;
               }
            }

            // CANCEL is hop by hop and should not be dispatched unless
            // the parent transaction was the originator of the CANCEL
            // request
            else if(!mIsCanceled)
            {
                int tempCseq;
                UtlString method;
                incomingMessage.getCSeqField(&tempCseq, &method);
                if(method.compareTo(SIP_CANCEL_METHOD) == 0)
                {
                    shouldDispatch = FALSE;
                }
            }
        }

#       ifdef TEST_PRINT
        osPrintf("SipTransaction::handleChildIncoming %p check response %d\n",
                 this, responseCode );
#       endif

        if(responseCode < SIP_TRYING_CODE)
        {
            // What is this????
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::handleChildIncoming dropped invalid response code: %d",
                          responseCode);
        }

        // 100 Trying is hop by hop do not forward it
        else if(responseCode == SIP_TRYING_CODE)
        {
            // no op
        }

        // If this is a successful 2XX or provisional response
        // forward it immediately
        // Once a final response is sent we no longer
        // send provisional responses, but we still send 2XX
        // class responses
        else if(   responseCode < SIP_3XX_CLASS_CODE
                && (   mpLastFinalResponse == NULL
                    || responseCode >= SIP_2XX_CLASS_CODE
                    )
                )
        {
            // If this is a server transaction for which the
            // response must be forwarded up stream
            if(mIsServerTransaction)
            {
                // If this is the
                // Forward immediately
                SipMessage response(incomingMessage);
                response.removeLastVia();
                response.resetTransport();
                response.clearDNSField();
#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleChildIncoming immediately forwarding %d response\n",
                    responseCode);
#endif
                handleOutgoing(response,
                                userAgent,
                                transactionList,
                                relationship,
                                mpTransport);
            }

            // If we got a good response for which forking
            // or recursion should be canceled
            if(mpParentTransaction == NULL)
            {
                // Got a success
                if(responseCode >= SIP_2XX_CLASS_CODE)
                {
                    // We are done - cancel all the outstanding requests
                    cancelChildren(userAgent,
                                   transactionList);
                }
            }

            // Keep track of the fact that we dispatched a final
            // response (but not for 1xx responses where xx > 00)
            if(   shouldDispatch
               && responseCode >= SIP_2XX_CLASS_CODE
               )
            {
#               ifdef TEST_PRINT
                osPrintf("SipTransaction::handleChildIncoming %p"
                         " should dispatch final response %d\n",
                         this, __LINE__);
#               endif
                mDispatchedFinalResponse = TRUE;
            }

            // This should not occur.  All 2xx class message for which
            // there is no parent server transaction should get dispatched
            else if(mpParentTransaction == NULL)
            {
               // xmlscott: despite the comment above, this happens a lot, 
               //           and seems to not always be bad,
               //           so I'm changing the priority to get it out of
               //           the logs.
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming "
                              "%d response with parent client transaction NOT dispatched",
                              responseCode);
            }
        }
        else
        {
            if (responseCode < SIP_2XX_CLASS_CODE)
            {
                // no op
            }
            // 3XX class responses
            else if(responseCode <= SIP_4XX_CLASS_CODE)
            {

                // Recursion is handled by the child
#               ifdef TEST_PRINT
                osPrintf( "SipTransaction::handleChildIncoming %p"
                         " 3XX response - should not dispatch\n", this);
#               endif

                // Wait until all the children have been searched
                // before dispatching
                shouldDispatch = FALSE;
            }
            // 4XX & 5XX class responses
            else if(responseCode <= SIP_6XX_CLASS_CODE)
            {
                // See if there are other outstanding child transactions

                // If there are none and no more to recurse find the
                // best result

                // If there are more to recurse do so.

                // Wait until all the children have been searched
                // before dispatching
                shouldDispatch = FALSE;
            }
            // 6XX class responses
            else
            {
                if(mIsServerTransaction)
                {
                    if(mpParentTransaction)
                    {
                        OsSysLog::add(FAC_SIP, PRI_ERR,
                                      "SipTransaction::handleChildIncoming"
                                      " 6XX server transaction is not top most parent"
                                      );
                    }

                    // Cancel any outstanding child transactions
                    cancelChildren(userAgent, transactionList);

                    // If they are all canceled or timed out, send back the 6xx
                    // response
                }

                // Wait until all the children have been searched
                // before dispatching
                shouldDispatch = FALSE;
            }

#           ifdef TEST_PRINT
            osPrintf( "SipTransaction::handleChildIncoming %p"
                     " response=%d parent=%p final=%p dispatched=%d \n"
                     ,this, responseCode, mpParentTransaction,
                     mpLastFinalResponse, mDispatchedFinalResponse
                     );
#           endif

            // If this is the server transaction and we have not
            // yet sent back a final response check what is next
            if(   (   mIsServerTransaction
                   && mpLastFinalResponse == NULL
                   )
               // or if this is the parent client transaction on a UA
               // and we have not yet dispatched the final response
               || (   mpParentTransaction == NULL
                   && ! mIsServerTransaction
                   && mpLastFinalResponse == NULL
                   && ! mDispatchedFinalResponse
                   )
               )
            {
#               ifdef TEST_PRINT
                UtlString dumpString;
                osPrintf( "SipTransaction::handleChildIncoming %p UAS/nofinal | UAC/parent\n", this );
                dumpTransactionTree(dumpString,FALSE);
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildIncoming\n%s",
                              dumpString.data());
#               else
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildIncoming %p", this );
#               endif

                if(mpParentTransaction)
                {
                    OsSysLog::add(FAC_SIP, PRI_ERR,
                                  "SipTransaction::handleChildIncoming %p "
                                  "server transaction is not top most parent", this);
                }

                // See if there is anything to sequentially search
                // startSequentialSearch returns TRUE if something
                // is still searching or it starts the next sequential
                // search
                if(startSequentialSearch(userAgent, transactionList, mpTransport))
                {
                }

                // Special case for when there is no server transaction
                // The parent client transaction, when it first gets a
                // 3xx response has no children, so we need to create them
                else if(   mChildTransactions.isEmpty()
                        && recurseChildren(userAgent, transactionList) // true if something started
                        )
                {
#                   ifdef TEST_PRINT
                    osPrintf("SipTransaction::handleChildIncoming %p creating children for 3XX\n",
                             this);
#                   endif
                }

                // Not waiting for outstanding transactions to complete
                else
                {
                    SipMessage bestResponse;
                    if(findBestResponse(bestResponse))
                    {
#                       ifdef TEST_PRINT
                        osPrintf("SipTransaction::handleChildIncoming %p sending best response\n",
                                 this);
#                       endif

                        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                        {
                           int bestResponseCode = bestResponse.getResponseStatusCode();
                           UtlString callId;
                           bestResponse.getCallIdField(&callId);
                           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                         // Format the Call-Id so it looks like
                                         // a header line in a SIP message,
                                         // so log processors see it.
                                         "SipTransaction::handleChildIncoming: "
                                         "response %d for \r\nCall-Id: %s\r",
                                         bestResponseCode, callId.data());
                        }

                        if(mIsServerTransaction)
                        {
                            handleOutgoing(bestResponse,
                                            userAgent,
                                            transactionList,
                                            MESSAGE_FINAL,
                                            mpTransport);
                        }

                        if(!mDispatchedFinalResponse)
                        {
                            // Get the best message out to be dispatched.
                            if(delayedDispatchedMessage)
                            {
                                delete delayedDispatchedMessage;
                                delayedDispatchedMessage = NULL;
                            }
                            delayedDispatchedMessage =
                                new SipMessage(bestResponse);

#                           ifdef DISPATCH_DEBUG
                            int delayedResponseCode =
                               delayedDispatchedMessage->getResponseStatusCode();
                            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildIncoming %p " 
                                          "delayed dispatch of %d\n",
                                          this, delayedResponseCode );
#                           endif
#                           ifdef TEST_PRINT
                            osPrintf("SipTransaction::handleChildIncoming %p "
                                     "should dispatch delayed message %d\n",
                                     this, __LINE__);
#                           endif
                            mDispatchedFinalResponse = TRUE;
                        }
                    }
                }
            }
        }

        // The response itself is getting dispatched
        if(   shouldDispatch
           && responseCode >= SIP_2XX_CLASS_CODE
           )
        {
            // Keep track of the fact that we dispatched a final
            // response
            mDispatchedFinalResponse = TRUE;
#           ifdef TEST_PRINT
            osPrintf("SipTransaction::handleChildIncoming %p should dispatch final response %d\n",
                     this, __LINE__);
#           endif

            if(delayedDispatchedMessage)
            {
                // This is probably a bug.  This should not
                // occur.  For now log some noise and
                // drop the delayed response, if this ever
                // occurs
                // xmlscott: lowered priority 
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "sipTransaction::handleChildIncoming %p dropping delayed response", this);
                delete delayedDispatchedMessage;
                delayedDispatchedMessage = NULL;
            }
        }
    }  // end new responses

    else if(relationship == MESSAGE_DUPLICATE)
    {
        // Proxy client transaction received a duplicate INVITE
        // response
        if(incomingMessage.isResponse() &&
           //mIsUaTransaction &&
           mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
        {
            int responseCode = incomingMessage.getResponseStatusCode();

            // The proxy must resend duplicate 2xx class responses
            // for reliability
            if(responseCode >= SIP_2XX_CLASS_CODE &&
                responseCode < SIP_3XX_CLASS_CODE)
            {
                // If there is more than one Via, send it upstream.
                // The calling UAC should resend the ACK, not the
                // proxy.
                UtlString dummyVia;
                if(incomingMessage.getViaField(&dummyVia, 1))
                {
                    SipTransaction* parent = getTopMostParent();
                    if(parent &&
                       parent->mIsServerTransaction)
                    {
                         OsSysLog::add(FAC_SIP, PRI_DEBUG, 
                                       "SipTransaction::handleChildIncoming "
                                       "proxy resending server transaction response %d",
                                       responseCode);
                        userAgent.sendStatelessResponse(incomingMessage);
                    }
                }

                // The ACK originated here, resend it
                else
                {
                    if(mpAck)
                    {
                        // Resend the ACK
                        SipMessage ackCopy(*mpAck);
                        ackCopy.removeLastVia();
                        userAgent.sendStatelessRequest(ackCopy,
                                                       mSendToAddress,
                                                       mSendToPort,
                                                       mSendToProtocol,
                                                       mBranchId);
                    }

                    // If this is a duplicate 2xx response and there is only
                    // one Via on the reponse, this UA should be the caller UAC.
                    // We should have an ACK that was sent for the original 2xx
                    // response.
                    else
                    {
                        OsSysLog::add(FAC_SIP, PRI_WARNING, 
                                      "SipTransaction::handleChildIncoming "
                                      "duplicate 2xx response received on UAC for INVITE "
                                      "with no ACK"
                                      );
                    }

                }

            }

            // INVITE with final response that failed
            else if(responseCode >= SIP_3XX_CLASS_CODE)
            {
                // For failed INVITE transactions, the ACK is
                // sent hop-by-hop
                if(mpAck)
                {
                    // Resend the ACK
                    SipMessage ackCopy(*mpAck);
                    ackCopy.removeLastVia();
                    userAgent.sendStatelessRequest(ackCopy,
                                                   mSendToAddress,
                                                   mSendToPort,
                                                   mSendToProtocol,
                                                   mBranchId);
                }

                // No ACK for a duplicate failed response.  Something
                // is wrong.  The ACK should have been created locally
                // for the previous 3xx, 4xx, 5xx or 6xx final response
                else
                {
                    OsSysLog::add(FAC_SIP, PRI_CRIT, 
                                  "SipTransaction::handleChildIncoming "
                                  "duplicate final error response rcvd for INVITE with no ACK");
                }
            }
        }
    }

    return(shouldDispatch);
} // end handleChildIncoming

void SipTransaction::handleChildTimeoutEvent(SipTransaction& child,
                                    const SipMessage& outgoingMessage,
                                    SipUserAgent& userAgent,
                                    enum messageRelationship relationship,
                                    SipTransactionList& transactionList,
                                    int& nextTimeout,
                                    SipMessage*& delayedDispatchedMessage,
                                    SIPX_TRANSPORT_DATA *pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    if(mpParentTransaction)
    {
        // For now recurse.  We might be able to short cut this
        // and go straight to the top most parent
        mpParentTransaction->handleChildTimeoutEvent(child,
                                                     outgoingMessage,
                                                     userAgent,
                                                     relationship,
                                                     transactionList,
                                                     nextTimeout,
                                                     delayedDispatchedMessage,
                                                     mpTransport);
    }

    // Top most parent
    else
    {
#ifdef LOG_FORKING
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildTimeoutEvent found top most parent: %p", this);
#endif

        //if(mIsServerTransaction)
        {
            UtlBoolean isResponse = outgoingMessage.isResponse();
            UtlString method;
            outgoingMessage.getRequestMethod(&method);

            if(   ! isResponse
               && method.compareTo(SIP_ACK_METHOD) == 0
               )
            {
                OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::handleChildTimeoutEvent timeout of ACK");
            }

            else if(   ! isResponse
                    && method.compareTo(SIP_CANCEL_METHOD) == 0
                    )
            {
            }

            else if(   relationship == MESSAGE_DUPLICATE
                    && ! isResponse
                    )
            {
                // Check if we should still be trying
                if(nextTimeout > 0)
                {
                    // Still trying
                }

                //else if(nextTimeout == 0)
                //{
                    // We do not dispatch proxy transactions
                //    nextTimeout = -1;
                //}

                // This transaction is done or has given up
                // See if we should start the next sequential search
                else
                {
                    // We do not dispatch proxy transactions
                    nextTimeout = -1;

#ifdef TEST_PRINT
                    osPrintf("SipTransaction::handleChildTimeoutEvent %d %p\n",__LINE__, this );
                    UtlString dumpString;
                    dumpTransactionTree(dumpString,FALSE);
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "sipTransaction::handleChildTimeoutEvent\n%s",
                        dumpString.data());
#else
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "sipTransaction::handleChildTimeoutEvent %p", this );
#endif

                    if(startSequentialSearch(userAgent, transactionList, mpTransport))
                    {
#ifdef LOG_FORKING
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildTimeoutEvent %p starting/still searching", this);
#endif
                    }

                    // Not waiting for outstanding transactions to complete
                    // and we have not yet sent a final response
                    else if(mpLastFinalResponse == NULL)
                    {
                        SipMessage bestResponse;
                        UtlBoolean foundBestResponse = findBestResponse(bestResponse);
                        // 2XX class responses are sent immediately so we
                        //  should not send it again
                        int bestResponseCode = bestResponse.getResponseStatusCode();
                        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                        {
                           UtlString callId;
                           bestResponse.getCallIdField(&callId);
                           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                         // Format the Call-Id so it looks like
                                         // a header line in a SIP message,
                                         // so log processors see it.
                                         "SipTransaction::handleChildTimeoutEvent: response %d for \r\nCall-Id: %s\r",
                                         bestResponseCode, callId.data());
                        }

                        // There is nothing to send if this is not a server transaction
                        // (this is the top most parent, if it is a client transaction
                        // the response gets dispatched).
                        if(   bestResponseCode >= SIP_3XX_CLASS_CODE
                           && mIsServerTransaction
                           && foundBestResponse
                           )
                        {
#ifdef LOG_FORKING
                            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildTimeoutEvent %p sending best response", this);
#endif
                            if (   (SIP_REQUEST_TIMEOUT_CODE == bestResponseCode)
                                && (!bestResponse.hasSelfHeader())
                                )
                            {
                               userAgent.setSelfHeader(bestResponse);
                            }
                            
                            handleOutgoing(bestResponse,
                                            userAgent,
                                            transactionList,
                                            MESSAGE_FINAL,
                                            mpTransport);
                        }
                        else
                        {
#ifdef LOG_FORKING
                            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleChildTimeoutEvent %p not sending %d best response",
                                          this, bestResponseCode);

#endif
                        }

                        if(foundBestResponse &&
                            !mDispatchedFinalResponse)
                        {
                            if(delayedDispatchedMessage)
                            {
                                delete delayedDispatchedMessage;
                                delayedDispatchedMessage = NULL;
                            }
                            delayedDispatchedMessage = new SipMessage(bestResponse);
#                           ifdef TEST_PRINT
                            osPrintf("SipTransaction::handleChildIncoming %p should dispatch final response %d\n",
                                     this, __LINE__);
#                           endif
                            mDispatchedFinalResponse = TRUE;
                        }
                    }

                }
            } // Request timeout

        } // server transaction

        //else
        //{
        //    OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::handleChildTimeoutEvent top most parent is not server transaction\n");
        //}
    }

}

UtlBoolean SipTransaction::startSequentialSearch(SipUserAgent& userAgent,
                                           SipTransactionList& transactionList,
                                           SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean childStillProceeding = FALSE;
    UtlBoolean startingNewSearch = FALSE;

#ifdef TEST_PRINT
    osPrintf("SipTransaction::startSequentialSearch %p\n", this );
#endif

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        if(   ! childTransaction->mIsCanceled
           && (   childTransaction->mTransactionState == TRANSACTION_CALLING
               || childTransaction->mTransactionState == TRANSACTION_PROCEEDING
               )
           )
        {
            // The child is not done
            childStillProceeding = TRUE;
#ifdef LOG_FORKING
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p still proceeding",
                this, childTransaction);
#endif
        }

        else if( childTransaction->mIsRecursing )
        {
            // See if the grand children or decendants are still searching
            if(childTransaction->startSequentialSearch(userAgent,
                transactionList,
                mpTransport))
            {
                childStillProceeding = TRUE;
#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p decendent still proceeding",
                    this, childTransaction);
#endif
            }
            else
            {
#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p no longer proceeding",
                    this, childTransaction);
#endif
            }
        }

        // A child has completed and may be recursed
        else if(   ! childStillProceeding
                && (   childTransaction->mTransactionState == TRANSACTION_COMPLETE
                    || childTransaction->mTransactionState == TRANSACTION_CONFIRMED
                    )
                && ! mIsCanceled
                && ! childTransaction->mIsCanceled
                )
        {
#ifdef LOG_FORKING
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p completed, now recursing",
                    this, childTransaction);
#endif
            UtlBoolean recurseStartedNewSearch = childTransaction->recurseChildren(userAgent,
                    transactionList);
            if(!startingNewSearch)
            {
                startingNewSearch = recurseStartedNewSearch;
            }

            // Do not break out of the loop because we want
            // to check all the currently proceeding transaction
            // to see if we should recurse.
#ifdef LOG_FORKING
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p startingNewSearch: %s",
                this, childTransaction, startingNewSearch ? "True" : "False");
#endif
        }

        // If there is another sequential search to kick off on this
        // parent
        else if(   ! childStillProceeding
                && ! startingNewSearch
                && childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED
                && ! mIsCanceled
                && ! childTransaction->mIsCanceled
                )
        {
            UtlBoolean recurseStartedNewSearch = FALSE;

            if(mpDnsSrvRecords)
            {
                recurseStartedNewSearch  = recurseDnsSrvChildren(userAgent, transactionList, mpTransport);
            }
            else
            {
                recurseStartedNewSearch = recurseChildren(userAgent, transactionList);
            }

            if(!startingNewSearch)
            {
                startingNewSearch = recurseStartedNewSearch;
            }
#ifdef LOG_FORKING
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p child: %p starting sequential startingNewSearch: %s",
                    this, childTransaction,
                    startingNewSearch ? "True" : "False");
#endif
            if( recurseStartedNewSearch )
            {
                break;
            }
            else
            {
#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p failed to find child to transaction to pursue", this);
#endif
            }
        }
    }

    mIsRecursing = childStillProceeding || startingNewSearch;
#ifdef LOG_FORKING
#ifdef TEST_PRINT
    osPrintf("SipTransaction::startSequentialSearch  %p returning: %s childStillProceeding: %s startingNewSearch:%s\n",
                  this, mIsRecursing ? "True" : "False",
                  childStillProceeding ? "True" : "False",
                  startingNewSearch ? "True" : "False");
#endif

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::startSequentialSearch %p returning: %s childStillProceeding: %s startingNewSearch:%s",
        this, mIsRecursing ? "True" : "False",
        childStillProceeding ? "True" : "False",
        startingNewSearch ? "True" : "False");
#endif
    return(mIsRecursing);
}

UtlBoolean SipTransaction::recurseDnsSrvChildren(SipUserAgent& userAgent,
                                                 SipTransactionList& transactionList,
                                                 SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    // If this is a client transaction requiring DNS SRV lookup
    // and we need to create the children to recurse
    if(!mIsServerTransaction &&
        !mIsDnsSrvChild &&
        mpDnsSrvRecords == NULL &&
        mpRequest &&  // only applicable to requests not sent
        mpLastFinalResponse == NULL && // should be no response yet
        mChildTransactions.isEmpty())
    {
        if(mSendToAddress.isNull())
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::recurseDnsSrvChildren no send address");
        }
        else if(mTransactionState < TRANSACTION_CONFIRMED)
        {
            mTransactionState = TRANSACTION_CONFIRMED;

            // Do the DNS SRV lookup for the request destination
            mpDnsSrvRecords = SipSrvLookup::servers(mSendToAddress.data(),
                                                "sip",
                                                 mSendToProtocol,
                                                 mSendToPort,
                                                 this->getRequest()->getLocalIp().data());

            // HACK:
            // Add a via to this request so when we set a timer it is
            // identified (by branchId) which transaction it is related to
            if(mpRequest)
            {
                // This via should never see the light of day (or rather
                // the bits of the network).
                mpRequest->addVia("127.0.0.1",
                       9999,
                       "UNKNOWN",
                       mBranchId.data());
            }

            // Set the transaction expires timeout for the DNS SRV parent
            int expireSeconds = mExpires;
            int maxExpires = userAgent.getDefaultExpiresSeconds();

            // Non-INVITE transactions timeout sooner
            if(this->mRequestMethod.compareTo(SIP_INVITE_METHOD) != 0)
            {
                maxExpires = (userAgent.getSipStateTransactionTimeout())/1000;
            }

            if(expireSeconds <= 0)
            {
                if(   mpParentTransaction
                   && mpParentTransaction->isChildSerial())
                {
                    expireSeconds = userAgent.getDefaultSerialExpiresSeconds();

                }

                else
                {
                    expireSeconds = maxExpires;
                }
            }

            // Make sure the expiration is not longer than
            // the maximum length of time we keep a transaction around
            if(expireSeconds > maxExpires)
            {
                expireSeconds = maxExpires;
            }

            // Set the transaction for the timeout message to make
            // it easier to find the transaction when the timer fires.
            mpRequest->setTransaction(this);

            // Keep separate copy for the timer
            SipMessage* pRequestMessage = NULL;
            pRequestMessage = new SipMessage(*mpRequest);
            
            SipMessageEvent* expiresEvent = 
                new SipMessageEvent(pRequestMessage, 
                                    SipMessageEvent::TRANSACTION_EXPIRATION);

            OsMsgQ* incomingQ = userAgent.getMessageQueue();
            OsTimer* expiresTimer = new OsTimer(incomingQ, 
                    (intptr_t)expiresEvent);
                    
            mTimers.append(expiresTimer);
#ifdef TEST_PRINT
            osPrintf("SipTransaction::recurseDnsSrvChildren added timer %p to timer list.\n",
                expiresTimer);
#endif

            OsTime expiresTime(expireSeconds, 0);
            expiresTimer->oneshotAfter(expiresTime);

            if (pTransport)
            {
                SipTransaction* childTransaction =
                    new SipTransaction(mpRequest,
                                        TRUE, // outgoing
                                        mIsUaTransaction); // same as parent

                if(childTransaction)
                {
                    // Set the q values of the child based upon the parent
                    // As DNS SRV is recursed serially the Q values are decremented
                    // by a factor of the record index

                    // Inherit the expiration from the parent
                    childTransaction->mExpires = mExpires;

                    // Mark this as a DNS SRV child
                    childTransaction->mIsDnsSrvChild = TRUE;

                    childTransaction->mIsBusy = mIsBusy;

                    // Add it to the list
                    transactionList.addTransaction(childTransaction);

                    // Link it in to this parent
                    this->linkChild(*childTransaction);
                }

            }
            else if(mpDnsSrvRecords)
            {
                int numSrvRecords = 0;
                int maxSrvRecords = userAgent.getMaxSrvRecords();

                // Create child transactions for each SRV record
                // up to the maximum
                while(numSrvRecords < maxSrvRecords &&
                    mpDnsSrvRecords[numSrvRecords].isValidServerT())
                {
                    SipTransaction* childTransaction =
                        new SipTransaction(mpRequest,
                                            TRUE, // outgoing
                                            mIsUaTransaction); // same as parent

                    mpDnsSrvRecords[numSrvRecords].
                       getIpAddressFromServerT(childTransaction->mSendToAddress);

                    childTransaction->mSendToPort =
                        mpDnsSrvRecords[numSrvRecords].getPortFromServerT();

                    childTransaction->mSendToProtocol =
                        mpDnsSrvRecords[numSrvRecords].getProtocolFromServerT();

#                        ifdef ROUTE_DEBUG
                         {
                            UtlString protoString;
                            SipMessage::convertProtocolEnumToString(childTransaction->mSendToProtocol,
                                                                    protoString);
                            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::recurseDnsSrvChildren "
                                          "new DNS SRV child %s:%d via '%s'",
                                          childTransaction->mSendToAddress.data(),
                                          childTransaction->mSendToPort,
                                          protoString.data());
                         }
#                        endif
                    // Do not create child for unsupported protocol types
                    if(childTransaction->mSendToProtocol ==
                        OsSocket::UNKNOWN)
                    {
                        maxSrvRecords++;
                        delete childTransaction;
                        childTransaction = NULL;
                    }

                    if(childTransaction)
                    {
                        // Set the q values of the child based upon the parent
                        // As DNS SRV is recursed serially the Q values are decremented
                        // by a factor of the record index
                        childTransaction->mQvalue = mQvalue - numSrvRecords * 0.0001;

                        // Inherit the expiration from the parent
                        childTransaction->mExpires = mExpires;

                        // Mark this as a DNS SRV child
                        childTransaction->mIsDnsSrvChild = TRUE;

                        childTransaction->mIsBusy = mIsBusy;

                        // Add it to the list
                        transactionList.addTransaction(childTransaction);

                        // Link it in to this parent
                        this->linkChild(*childTransaction);
                    }

                    numSrvRecords++;
                }
            }

            // We got no DNS SRV records back
            else
            {
                OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::recurseDnsSrvChildren no DNS SRV records");
            }
        }
    }

    UtlBoolean childRecursed = FALSE;
    UtlBoolean childRecursing = FALSE;
    if(!mIsServerTransaction &&
        !mIsDnsSrvChild &&
        mpDnsSrvRecords &&
        mpRequest)
    {
        UtlSListIterator iterator(mChildTransactions);
        SipTransaction* childTransaction = NULL;

        while((childTransaction = (SipTransaction*) iterator()) &&
             !childRecursed &&
             !childRecursing)
        {
            if(childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED)
            {
                // Make a local copy to modify and send
                SipMessage recursedRequest(*mpRequest);

                // Clear the address and port of the previous send
                // of the parent request.
                recursedRequest.removeLastVia(); // the fake via for identifying this TX
                recursedRequest.resetTransport();
                recursedRequest.clearDNSField();

#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipTransaction::recurseDnsSrvChildren %p sending child transaction request %s:%d protocol: %d",
                              this,
                              childTransaction->mSendToAddress.data(),
                              childTransaction->mSendToPort,
                              childTransaction->mSendToProtocol);
#endif
                // Start the transaction by sending its request
                if(childTransaction->handleOutgoing(recursedRequest,
                                                 userAgent,
                                                 transactionList,
                                                 MESSAGE_REQUEST,
                                                 mpTransport))
                {
                    childRecursed = TRUE;
                }

            }

            // If there is a child transaction that is currently
            // being pursued, do not start any new searches
            else if((childTransaction->mTransactionState == TRANSACTION_CALLING ||
                childTransaction->mTransactionState == TRANSACTION_PROCEEDING) &&
                !childTransaction->mIsCanceled)
            {
                childRecursing = TRUE;
#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseDnsSrvChildren %p still pursing", this);
#endif
            }

            // This parent is not canceled (implicit) and we found a non-canceled
            // DNS SRV child with any sort of response, so there is not need to
            // recurse.  We have a DNS SRV child that has succeeded (at least in
            // as much as getting a response).
            else if(!childTransaction->mIsCanceled &&
                (childTransaction->mpLastProvisionalResponse ||
                childTransaction->mpLastFinalResponse))
            {
                break;
            }

            else
            {
#ifdef LOG_FORKING
                UtlString state;
                SipTransaction::getStateString(childTransaction->mTransactionState,
                    state);
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseDnsSrvChildren %p transaction not recursed state: %s", this,
                    state.data());
#endif
            }
        }
    }

    if(childRecursed) mIsRecursing = TRUE;
    return(childRecursed);
}

UtlBoolean SipTransaction::recurseChildren(SipUserAgent& userAgent,
                                   SipTransactionList& transactionList)
{
    UtlBoolean childRecursed = FALSE;

#ifdef LOG_FORKING
#ifdef TEST_PRINT
    osPrintf("SipTransaction::recurseChildren %p\n", this);
#endif

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p", this);
#endif

    if(mpRequest == NULL)
    {
        UtlString transactionString;
        toString(transactionString, TRUE);
        OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::recurseChildren NULL mpResponse\n======>\n%s\n======>",
            transactionString.data());
    }

    if(mpLastFinalResponse && mpRequest)
    {
        SipTransaction* childTransaction = NULL;
        int responseCode = mpLastFinalResponse->getResponseStatusCode();

#       ifdef TEST_PRINT
        osPrintf("SipTransaction: lastfinalsresponse & request forking enabled: %d ce: %d\n"
                 ,userAgent.isForkingEnabled(), mChildTransactions.isEmpty()
                 );
#       endif
        // If this is a client transaction for which we received
        // a 3XX response on which recursion makes sense, create
        // the child transactions
        if(   userAgent.isForkingEnabled()
           && responseCode >= SIP_3XX_CLASS_CODE
           && responseCode < SIP_4XX_CLASS_CODE
           && mChildTransactions.isEmpty()
           )
        {
            int contactIndex = 0;
            UtlString contactString;
            while( mpLastFinalResponse->getContactField(contactIndex,
                                                        contactString)
                  )
            {
                Url contactUrl(contactString);

                // Make sure we do not add the contact twice and
                // we have not already pursued this contact
                if(!isUriChild(contactUrl) &&
                   !isUriRecursed(contactUrl))
                {
                    childTransaction = new SipTransaction(mpRequest,
                        TRUE, // outgoing
                        mIsUaTransaction); // proxy transactions

#ifdef LOG_FORKING
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p adding child %p for contact: %s",
                                  this, childTransaction, contactString.data());
#ifdef TEST_PRINT
                    osPrintf( "SipTransaction::recurseChildren adding transaction %p for contact: %s\n",
                             childTransaction, contactString.data());
#endif
#endif
                    // Add it to the list of all transactions
                    transactionList.addTransaction(childTransaction);

                    // Set the URI of the copy to that from the contact
                    contactUrl.getUri(childTransaction->mRequestUri);

                    // Set the Q value and Expires value
                    UtlString qString;
                    UtlString expiresString;
                    double qValue = 1.0;
                    if(contactUrl.getFieldParameter("q", qString))
                        qValue = atof(qString.data());
                    int expiresSeconds = -1;
                    if((mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0) &&
                        contactUrl.getHeaderParameter("expires", expiresString))
                    {
                        // All digits, the format is relative seconds
                        if(Url::isDigitString(expiresString.data()))
                        {
                            expiresSeconds = atoi(expiresString.data());
                        }

                        // Alphanumeric, it is an HTTP absolute date
                        else
                        {
                            expiresSeconds = OsDateTime::convertHttpDateToEpoch(expiresString.data());
                            OsTime time;
                            OsDateTime::getCurTimeSinceBoot(time);
                            expiresSeconds -= time.seconds();
                            //expiresSeconds -= OsDateTime::getSecsSinceEpoch();
                        }
                    }

                    // Set the values of the child
                    childTransaction->mQvalue = qValue;
                    childTransaction->mExpires = expiresSeconds;

                    // Link it in to this parent
                    this->linkChild(*childTransaction);
                }

                // We have all ready recursed this contact
                else
                {
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p already recursed: %s",
                        this, contactString.data());
                }

                contactIndex++;
            } // end for each contact
        }

        double nextQvalue = -1.0;
        int numRecursed = 0;
        UtlSListIterator iterator(mChildTransactions);
        while ((childTransaction = (SipTransaction*) iterator()))
        {
            // Until a request is successfully sent, reset the
            // current Q value at which transactions of equal
            // value are searched in parallel
            if(numRecursed == 0) nextQvalue = -1.0;

            if(childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED)
            {
                double qDelta = nextQvalue - childTransaction->mQvalue;
                double qDeltaSquare = qDelta * qDelta;

#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p qDelta: %f qDeltaSquare: %f mQvalue: %f",
                              this, qDelta, qDeltaSquare, childTransaction->mQvalue);
#ifdef TEST_PRINT
                osPrintf( "SipTransaction::recurseChildren qDelta: %f qDeltaSquare: %f mQvalue: %f\n",
                              qDelta, qDeltaSquare, childTransaction->mQvalue);
#endif
#endif

                if(nextQvalue <= 0.0 ||
                   qDeltaSquare < MIN_Q_DELTA_SQUARE)
                {
                    nextQvalue = childTransaction->mQvalue;

#ifdef LOG_FORKING
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p should recurse child: %p q: %f",
                                  this, childTransaction, childTransaction->mQvalue);
#ifdef TEST_PRINT
                    osPrintf( "SipTransaction::recurseChildren parent: %p should recurse child: %p q: %f\n",
                                  this, childTransaction, childTransaction->mQvalue);
#endif
#endif

                    // Make a local copy to modify and send
                    SipMessage recursedRequest(*mpRequest);

                    // Clear the address and port of the previous send
                    // of the parent request.
                    recursedRequest.removeLastVia();
                    recursedRequest.resetTransport();
                    recursedRequest.clearDNSField();

                    // If there was a loose route pop it off
                    // The assumption is that this was previously routed
                    // to the redirect server.  Perhaps we can get the
                    // parent's parent's request to use here instead.  I just
                    // cannot work it out in my head right now
                    UtlString routeUri;
                    recursedRequest.getRouteUri(0, &routeUri);
                    Url routeUrlParser(routeUri);
                    UtlString dummyValue;
                    UtlBoolean nextHopLooseRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
                    if(nextHopLooseRoutes)
                    {
                        recursedRequest.removeRouteUri(0, &routeUri);
                    }

                    // Correct the URI of the request to be the recursed URI
                    recursedRequest.setSipRequestFirstHeaderLine(mRequestMethod,
                        childTransaction->mRequestUri, SIP_PROTOCOL_VERSION);

                    // Decrement max-forwards
                    int maxForwards;
                    if(!recursedRequest.getMaxForwards(maxForwards))
                    {
                        recursedRequest.setMaxForwards(userAgent.getMaxForwards() - 1);
                    }
                    else
                    {
                        recursedRequest.setMaxForwards(maxForwards - 1);
                    }

#ifdef LOG_FORKING
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p sending child transaction request", this);
#ifdef TEST_PRINT
                    osPrintf( "SipTransaction::recurseChildren sending child transaction request\n");
#endif
#endif
                    // Start the transaction by sending its request
                    if(childTransaction->handleOutgoing(recursedRequest,
                                                     userAgent,
                                                     transactionList,
                                                     MESSAGE_REQUEST,
                                                     mpTransport))
                    {

                        numRecursed++;
                        // Recursing is TRUE
                        childRecursed = TRUE; // Recursion disabled
                    }
                }

                else
                {
#ifdef LOG_FORKING
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p nextQvalue: %f qDeltaSquare: %f", this,
                        nextQvalue, qDeltaSquare);
#ifdef TEST_PRINT
                    osPrintf( "SipTransaction::recurseChildren nextQvalue: %f qDeltaSquare: %f\n",
                                  nextQvalue, qDeltaSquare);
#endif
#endif
                }
            }

            // If there is a child transaction that is currently
            // being pursued, do not start any new searches
            else if((childTransaction->mTransactionState == TRANSACTION_CALLING ||
                childTransaction->mTransactionState == TRANSACTION_PROCEEDING) &&
                !childTransaction->mIsCanceled)
            {
                nextQvalue = childTransaction->mQvalue;
#ifdef LOG_FORKING
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p still pursing", this);
#ifdef TEST_PRINT
                osPrintf( "SipTransaction::recurseChildren still pursing\n");
#endif
#endif
            }

            else
            {
#ifdef LOG_FORKING
                UtlString state;
                SipTransaction::getStateString(childTransaction->mTransactionState,
                    state);
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::recurseChildren %p transaction not recursed state: %s",
                              this, state.data());
#ifdef TEST_PRINT
                osPrintf( "SipTransaction::recurseChildren transaction not recursed state: %s\n",
                         state.data());
#endif
#endif
            }

            // Optionally we only look at the first contact
            // for 300 response (not 300 class e.g. 302)
            if(userAgent.recurseOnlyOne300Contact() &&
                responseCode == SIP_MULTI_CHOICE_CODE) break;
        }
    }

    if(childRecursed) mIsRecursing = TRUE;
    return(childRecursed);
}

UtlBoolean SipTransaction::findBestResponse(SipMessage& bestResponse)
{
    //osPrintf("SipTransaction::findBestResponse\n");
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean responseFound = FALSE;
    SipMessage* childResponse = NULL;
    int bestResponseCode = -1;
    int childResponseCode;
    UtlString authField;
    UtlBoolean foundChild = FALSE;
#ifdef TEST_PRINT
    osPrintf("SipTransaction::findBestResponse %p\n", this);
#endif

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // Check the child's decendents first
        // Note: we need to check the child's children even if this child
        // has no response.
        foundChild = childTransaction->findBestResponse(bestResponse);
        if(foundChild) responseFound = TRUE;

        childResponse = childTransaction->mpLastFinalResponse;
        if(childResponse)
        {
            bestResponseCode = bestResponse.getResponseStatusCode();

            childResponseCode =
                childResponse->getResponseStatusCode();

            if((bestResponseCode == HTTP_UNAUTHORIZED_CODE ||
                bestResponseCode == HTTP_PROXY_UNAUTHORIZED_CODE) &&
                (childResponseCode == HTTP_UNAUTHORIZED_CODE ||
                childResponseCode == HTTP_PROXY_UNAUTHORIZED_CODE))
            {
                // Aggregate Authenticate fields

                // Get the proxy authenticate challenges
                int authIndex = 0;
                while(childResponse->getAuthenticationField(authIndex,
                    HttpMessage::PROXY, authField))
                {
                    bestResponse.addAuthenticationField(authField.data(),
                                                        HttpMessage::PROXY);
                    authIndex++;
                }

                // Get the UA server authenticate challenges
                authIndex = 0;
                while(childResponse->getAuthenticationField(authIndex,
                    HttpMessage::SERVER, authField))
                {
                    bestResponse.addAuthenticationField(authField.data(),
                                                        HttpMessage::SERVER);
                    authIndex++;
                }
            }

            // 401 & 407 are better than any other 4xx, 5xx or 6xx
            else if(bestResponseCode >= SIP_4XX_CLASS_CODE &&
                (bestResponseCode != HTTP_UNAUTHORIZED_CODE &&
                bestResponseCode != HTTP_PROXY_UNAUTHORIZED_CODE) &&
                (childResponseCode == HTTP_UNAUTHORIZED_CODE ||
                childResponseCode == HTTP_PROXY_UNAUTHORIZED_CODE))
            {
                bestResponse = *(childResponse);

                bestResponse.removeLastVia();
                bestResponse.resetTransport();
                bestResponse.clearDNSField();
                responseFound = TRUE;
            }

            // 3xx is better than 4xx
            else if(bestResponseCode >= SIP_4XX_CLASS_CODE &&
                childResponseCode <= SIP_4XX_CLASS_CODE &&
                childTransaction->mChildTransactions.isEmpty())
            {
                // An untryed 3xx response
                bestResponse = *(childResponse);

                bestResponse.removeLastVia();
                bestResponse.resetTransport();
                bestResponse.clearDNSField();
                responseFound = TRUE;
            }

            // have not found a response that is not recursed yet
            // Ignore any 487s as these are responses to forking.
            // CANCELs from the client are responded to on the
            // server transaction upon receipt.
            else if(!responseFound &&
                childResponse &&
                childTransaction->mChildTransactions.isEmpty() &&
                childResponseCode != SIP_REQUEST_TERMINATED_CODE)
            {
                bestResponse = *(childResponse);

                // Not suppose to return 503 unless we know that
                // there is absolutely no way to reach the end point
                if(childResponseCode == SIP_SERVICE_UNAVAILABLE_CODE)
                {
                    bestResponse.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                        SIP_SERVER_INTERNAL_ERROR_CODE,
                        SIP_SERVER_INTERNAL_ERROR_TEXT);
                }

                bestResponse.removeLastVia();
                bestResponse.resetTransport();
                bestResponse.clearDNSField();

                // For now we do not support forking, just return
                // the first and should be the only final response
                responseFound = TRUE;
                //break;
            }
        }

    }

    // We have made it to the top and there are no responses
    if(!responseFound &&
        mpParentTransaction == NULL)
    {
        if(mpRequest)
        {
            bestResponse.setResponseData(mpRequest,
                SIP_REQUEST_TIMEOUT_CODE,
                SIP_REQUEST_TIMEOUT_TEXT);
            responseFound = TRUE;
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::findBestResponse no request");
        }
    }

    if(responseFound)
    {
        const char* firstHeaderLine = bestResponse.getFirstHeaderLine();

        if(firstHeaderLine == NULL ||
            *firstHeaderLine == '\0')
        {
            if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString msgString;
                int msgLen;
                bestResponse.getBytes(&msgString, &msgLen);

            // We got a bad response
            OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipTransaction::findBestResponse invalid response:\n%s",
                        msgString.data());
#           ifdef TEST_PRINT
            osPrintf("SipTransaction::findBestResponse invalid response:\n%s",
                     msgString.data());
#           endif
            }
        }
    }


    return(responseFound);
}

UtlBoolean SipTransaction::doResend(SipMessage& resendMessage,
                                   SipUserAgent& userAgent,
                                   int& nextTimeout,
                                   SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    // Find out how many times we have tried
    nextTimeout = 0;
    int numTries = resendMessage.getTimesSent();
    OsSocket::IpProtocolSocketType protocol = resendMessage.getSendProtocol();
    int lastTimeout = resendMessage.getResendDuration();
    UtlString sendAddress;

    int sendPort;
    resendMessage.getSendAddress(&sendAddress, &sendPort);
    UtlBoolean sentOk = FALSE;

    // TCP only gets one try
    // UDP gets SIP_UDP_RESEND_TIMES tries

    if(protocol == OsSocket::UDP)
    {
        if(numTries < SIP_UDP_RESEND_TIMES)
        {
            // Try UDP again
            numTries++;
            if(userAgent.sendUdp(&resendMessage,
                sendAddress.data(), sendPort))
            {
                // Do this after the send so that
                // the log message is correct
                resendMessage.setTimesSent(numTries);

                // Schedule a time out
                if(lastTimeout <
                    userAgent.getFirstResendTimeout())
                {
                    nextTimeout =
                        userAgent.getFirstResendTimeout();
                }
                else if(lastTimeout <
                    userAgent.getLastResendTimeout())
                {
                    nextTimeout = lastTimeout * 2;
                }
                else
                {
                    nextTimeout =
                        userAgent.getLastResendTimeout();
                }

                resendMessage.setTimesSent(numTries);
                resendMessage.setResendDuration(nextTimeout);

                sentOk = TRUE;
            }
        }
    }

#ifdef SIP_TLS
    else if(protocol == OsSocket::TCP ||
       protocol == OsSocket::SSL_SOCKET)
#else
    else if(protocol == OsSocket::TCP)
#endif
    {
       if(numTries >= 1)
       {
            // we are done send back a transport error

            // Dispatch is called from above this method

            // Dispatch needs it own copy of the message
            //SipMessage* dispatchMessage = new SipMessage(resendMessage);

            // The TCP send failed, pass back an error
            //userAgent.dispatch(dispatchMessage,
            //                   SipMessageEvent::TRANSPORT_ERROR);
       }
       else
       {
           // Try TCP once
           numTries = 1;

           UtlBoolean sendOk;
           if(protocol == OsSocket::TCP)
           {
              sendOk = userAgent.sendTcp(&resendMessage,
                                         sendAddress.data(),
                                         sendPort);
           }
#ifdef SIP_TLS
           else if(protocol == OsSocket::SSL_SOCKET)
           {
              sendOk = userAgent.sendTls(&resendMessage,
                                         sendAddress.data(),
                                         sendPort);
           }
#endif
           else
           {
              sendOk = FALSE;
           }

           if(sendOk)
           {
               // Schedule a timeout
               nextTimeout =
                   userAgent.getReliableTransportTimeout();

               resendMessage.setTimesSent(numTries);
               resendMessage.setResendDuration(nextTimeout);
               resendMessage.setSendProtocol(protocol);

#ifdef TEST_PRINT
                if(resendMessage.getSipTransaction() == NULL)
                {
                    UtlString msgString;
                    int msgLen;
                    resendMessage.getBytes(&msgString, &msgLen);
                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipTransaction::doResend reschedule of request resend with NULL transaction",
                        msgString.data());
                    osPrintf("SipTransaction::doResend reschedule of request resend with NULL transaction\n",
                        msgString.data());
                }
#endif

               // Schedule a timeout for requests which do not
               // receive a response
               SipMessageEvent* resendEvent =
                    new SipMessageEvent(new SipMessage(resendMessage),
                                    SipMessageEvent::TRANSACTION_RESEND);

               OsMsgQ* incomingQ = userAgent.getMessageQueue();
               OsTimer* timer = new OsTimer(incomingQ,
                   (intptr_t)resendEvent);
               mTimers.append(timer);
#ifdef TEST_PRINT
               osPrintf("SipTransaction::doResend added timer %p to timer list.\n", timer);
#endif

               // Convert from mSeconds to uSeconds
               OsTime lapseTime(0,
                   nextTimeout * 1000);
               timer->oneshotAfter(lapseTime);


               sentOk = TRUE;
           }

           else
           {
               // Send failed send back the transport error

               // Dispatch is done from above this method

               // Dispatch needs it own copy of the message
               // SipMessage* dispatchMessage = new SipMessage(resendMessage);

                // The TCP send failed, pass back an error
                //userAgent.dispatch(dispatchMessage,
                //                   SipMessageEvent::TRANSPORT_ERROR);
           }
       }
    } // TCP
    else if (protocol == OsSocket::CUSTOM)
    {
        if(!pTransport->bIsReliable && numTries < SIP_UDP_RESEND_TIMES)
        {
            // Try again
            numTries++;
            if(userAgent.sendCustom(pTransport, &resendMessage, sendAddress.data(), sendPort))
            {
                // Do this after the send so that
                // the log message is correct
                resendMessage.setTimesSent(numTries);

                // Schedule a time out
                if(lastTimeout <
                    userAgent.getFirstResendTimeout())
                {
                    nextTimeout =
                        userAgent.getFirstResendTimeout();
                }
                else if(lastTimeout <
                    userAgent.getLastResendTimeout())
                {
                    nextTimeout = lastTimeout * 2;
                }
                else
                {
                    nextTimeout =
                        userAgent.getLastResendTimeout();
                }

                resendMessage.setTimesSent(numTries);
                resendMessage.setResendDuration(nextTimeout);

                sentOk = TRUE;
            }
        }
    }

    return(sentOk);
} // end doResend

UtlBoolean SipTransaction::handleIncoming(SipMessage& incomingMessage,
                                         SipUserAgent& userAgent,
                                         enum messageRelationship relationship,
                                         SipTransactionList& transactionList,
                                         SipMessage*& delayedDispatchedMessage,
                                         SIPX_TRANSPORT_DATA* pTransport)
{
    if (!mpTransport) mpTransport = pTransport;

    if(delayedDispatchedMessage)
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleIncoming delayedDispatchedMessage not NULL");
        delayedDispatchedMessage = NULL;
    }

    UtlBoolean shouldDispatch = FALSE;

    if(relationship == MESSAGE_UNKNOWN)
    {
        relationship = whatRelation(incomingMessage,
                                    TRUE);
    }


    // This is a message was already recieved once
    if(relationship == MESSAGE_DUPLICATE)
    {
        // Update the time stamp so that this message
        // does not get garbage collected right away.
        // This is so that rogue UAs that keep sending
        // the same message for a long (i.e. longer than
        // transaction timeout) period of time are ignored.
        // Otherwise this message looks like a new message
        // after a transaction timeout and the original
        // copy gets garbage collected.  We explicitly
        // do NOT touch the outgoing (i.e. sentMessages)
        // as we do not want to keep responding after
        // the transaction timeout.
        //previousMessage->touchTransportTime();

        // If it is a request resend the response if it exists
        if(!(incomingMessage.isResponse()))
        {
            SipMessage* response = NULL; //sentMessages.getResponseFor(message);
            UtlString method;
            incomingMessage.getRequestMethod(&method);
            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
                // Do nothing, we already have the ACK
            }
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
                // Resend the CANCEL response
                response = mpCancelResponse;
            }
            else
            {
                // Resend the final response if there is one
                // Otherwise resend the provisional response
                response = mpLastFinalResponse ?
                    mpLastFinalResponse : mpLastProvisionalResponse;
            }


#ifdef TEST_PRINT
            osPrintf("SipTransaction::handleIncoming duplicate REQUEST response\n");

            if(response)
            {
               osPrintf("response protocol: %d\n", response->getSendProtocol());
            }
#endif
            if(response)
            // No longer filter on reliable protocol as even if this
            // was TCP there could be a switch from TCP to UDP somewhere
            // in the proxy chain.  Hense we should resend anyway
            //&&  response->getSendProtocol() == OsSocket::UDP)
            {
                int sendProtocol = response->getSendProtocol();
                UtlString sendAddress;
                int sendPort;
                response->getSendAddress(&sendAddress, &sendPort);

                if(sendProtocol == OsSocket::UDP)
                {
                    userAgent.sendUdp(response, sendAddress.data(), sendPort);
                }
                else if(sendProtocol == OsSocket::TCP)
                {
                    userAgent.sendTcp(response, sendAddress.data(), sendPort);
                }
#ifdef SIP_TLS
                else if(sendProtocol == OsSocket::SSL_SOCKET)
                {
                    userAgent.sendTls(response, sendAddress.data(), sendPort);
                }
#endif
                else if (sendProtocol >= OsSocket::CUSTOM)
                {
                    userAgent.sendCustom(NULL, response, sendAddress.data(), sendPort);
                }

#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleIncoming resending response\n");
#endif
            }
        }

        // If it is an INVITE response, resend the ACK if it exists
        //
        else
        {
            int cSeq;
            UtlString seqMethod;
            incomingMessage.getCSeqField(&cSeq, &seqMethod);

            // We assume ACK will only exist if this was an INVITE
            // transaction.  We resend only if this is a
            // UA transaction.
            if (   !seqMethod.compareTo(SIP_CANCEL_METHOD)
                && mpAck
                && mIsUaTransaction
                )
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::handleIncoming resending ACK");
#endif
                int sendProtocol = mpAck->getSendProtocol();
                UtlString sendAddress;
                int sendPort;
                mpAck->getSendAddress(&sendAddress, &sendPort);

                if(sendProtocol == OsSocket::UDP)
                {
                    userAgent.sendUdp(mpAck, sendAddress.data(), sendPort);
                }
                else if(sendProtocol == OsSocket::TCP)
                {
                    userAgent.sendTcp(mpAck, sendAddress.data(), sendPort);
                }
#ifdef SIP_TLS
                else if(sendProtocol == OsSocket::SSL_SOCKET)
                {
                    userAgent.sendTls(mpAck, sendAddress.data(), sendPort);
                }
#endif
                else if (sendProtocol >= OsSocket::CUSTOM)
                {
                    userAgent.sendCustom(NULL, mpAck, sendAddress.data(), sendPort);
                }

#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleIncoming resent ACK\n");
#endif
            }
        }
    }

    // The first time we received this message
    else if(relationship == MESSAGE_FINAL)
    {
        if(mpAck)
        {
            int cSeq;
            UtlString seqMethod;
            incomingMessage.getCSeqField(&cSeq, &seqMethod);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleIncoming resending ACK for final response %s",
                          seqMethod.data()
                          );

            int sendProtocol = mpAck->getSendProtocol();
            UtlString sendAddress;
            int sendPort;
            mpAck->getSendAddress(&sendAddress, &sendPort);

            if(sendProtocol == OsSocket::UDP)
            {
                userAgent.sendUdp(mpAck, sendAddress.data(), sendPort);
            }
            else if(sendProtocol == OsSocket::TCP)
            {
                userAgent.sendTcp(mpAck, sendAddress.data(), sendPort);
            }
#ifdef SIP_TLS
            else if(sendProtocol == OsSocket::SSL_SOCKET)
            {
                userAgent.sendTls(mpAck, sendAddress.data(), sendPort);
            }
#endif
            else if (sendProtocol >= OsSocket::CUSTOM)
            {
                userAgent.sendCustom(NULL, mpAck, sendAddress.data(), sendPort);
            }
            mpAck->incrementTimesSent();
            mpAck->touchTransportTime();

            shouldDispatch = TRUE;

            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipTransaction::handleIncoming received final response,"
                          "sending existing ACK"
                          );
        }

        else
        {
            // If this is an error final response to an INVITE
            // We can automatically construct the ACK here:
            if(   mpRequest
               && (mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
               && (incomingMessage.getResponseStatusCode() >= SIP_3XX_CLASS_CODE)
               )
            {
                SipMessage ack;
                ack.setAckData(&incomingMessage,
                    mpRequest);

               // SDUA
               // is err code > 300 , set the DNS data files in the response
               // We may not need this any more as the ACK for error
               // responses is now done here
               UtlString protocol;
               UtlString address;
               UtlString port;
               if (mpRequest->getDNSField( &protocol , &address , &port))
               {
                   ack.setDNSField(protocol,
                                   address,
                                   port);
               }
#ifdef TEST_PRINT
               osPrintf("SipTransaction::handleIncoming %p sending ACK for error response\n", this);
#endif
               handleOutgoing(ack,
                              userAgent,
                              transactionList,
                              MESSAGE_ACK,
                              mpTransport);

               shouldDispatch = TRUE;
            }

            // Non INVITE response or 2XX INVITE responses for which
            // there is no ACK yet get dispatched.  The app layer must
            // generate the ACK for 2XX responses as it may contain
            // SDP
            else
            {
                shouldDispatch = TRUE;
            }
        }

        SipMessage* responseCopy = new SipMessage(incomingMessage);
        addResponse(responseCopy,
                    FALSE, // Incoming
                    relationship);

    } // End if new final response

    // Requests, provisional responses, Cancel response
    else
    {
       SipMessage* responseCopy = new SipMessage(incomingMessage);
        addResponse(responseCopy,
                    FALSE, // Incoming
                    relationship);

        if(relationship == MESSAGE_REQUEST &&
            mIsServerTransaction)
        {
            if(mpLastProvisionalResponse)
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleIncoming new request with and existing provisional response");
            }
            else if(mpLastFinalResponse)
            {
                OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::handleIncoming new request with and existing final response");
            }
            // INVITE transactions we can send trying to stop the resends
            else if(mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                // Create and send a 100 Trying
                SipMessage trying;
                trying.setTryingResponseData(&incomingMessage);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::handleIncoming sending trying response\n");
#endif
                handleOutgoing(trying,
                               userAgent,
                               transactionList,
                               MESSAGE_PROVISIONAL,
                               mpTransport);
            }
        }

        // If this transaction was marked as canceled
        // but we could not send the cancel until we
        // get a provisional response, we can now send
        // the CANCEL (we only send CANCEL for INVITEs).
        if(relationship == MESSAGE_PROVISIONAL &&
            !mIsServerTransaction &&
            mIsCanceled &&
            mpCancel == NULL && // have not already sent CANCEL
            mpRequest &&
            mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
        {
            SipMessage cancel;

            cancel.setCancelData(mpRequest);
#ifdef TEST_PRINT
            osPrintf("SipTransaction::handleIncoming sending cancel after receiving first provisional response\n");
#endif
            handleOutgoing(cancel,
                           userAgent,
                           transactionList,
                           MESSAGE_CANCEL,
                           mpTransport);
        }

        // Incoming CANCEL, respond and cancel children
        else if(mIsServerTransaction &&
            relationship == MESSAGE_CANCEL)
        {
            if(mpRequest)
            {
                UtlString reqMethod;
                mpRequest->getRequestMethod(&reqMethod);
                if(reqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
                   (mTransactionState == TRANSACTION_PROCEEDING ||
                   mTransactionState == TRANSACTION_CALLING))
                {
                    // Proxy transaction
                    if(!mIsUaTransaction)
                    {
                        cancelChildren(userAgent,
                                       transactionList);
                        shouldDispatch = FALSE;
                    }

                    // UA transaction
                    else
                    {
                        shouldDispatch = TRUE;
                    }

                    if(mpLastFinalResponse == NULL)
                    {
                        // I think this is wrong only the app. layer of a
                        // UAS should response with 487
                        // Respond to the server transaction INVITE
                        //SipMessage inviteResponse;
                        //inviteResponse.setResponseData(mpRequest,
                        //                    SIP_REQUEST_TERMINATED_CODE,
                        //                    SIP_REQUEST_TERMINATED_TEXT);
                        //handleOutgoing(inviteResponse, userAgent,
                        //    MESSAGE_FINAL);
                    }

                }
				else
				{
					// Erik Fournier 16/09/2005
					// The 200 response should only be sent here if the CANCEL request
					// is not going to be dispatched to the application layer.

					// Too late to cancel, non-INVITE request and
					// successfully canceled INVITEs all get a
					// 200 response to the CANCEL
					SipMessage cancelResponse;
					cancelResponse.setResponseData(&incomingMessage,
                                    SIP_OK_CODE,
                                    SIP_OK_TEXT);
#ifdef TEST_PRINT
					osPrintf("SipTransaction::handleIncoming sending CANCEL response\n");
#endif
					handleOutgoing(cancelResponse,
								   userAgent,
                                   transactionList,
                                   MESSAGE_CANCEL_RESPONSE,
                                   mpTransport);
				}
            }

            // No request to cancel
            else
            {
                // Send a transaction not found error
                SipMessage cancelError;
                cancelError.setBadTransactionData(&incomingMessage);
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipTransaction::handleIncoming transaction not found for CANCEL\n");
                handleOutgoing(cancelError,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL_RESPONSE,
                               mpTransport);
            }

        } // End cancel request

        else
        {
            // ACK received for server transaction
            if(mIsServerTransaction &&
               relationship == MESSAGE_ACK)
            {
                int responseCode = -1;
                if(mpLastFinalResponse)
                {
                    responseCode =
                        mpLastFinalResponse->getResponseStatusCode();
                }

                // If this INVITE transaction ended in an error final response
                // we do not forward the message via the clients transactions
                // the client generates its own ACK
                if(responseCode >= SIP_3XX_CLASS_CODE)
                {
                    shouldDispatch = FALSE;
                }

                // Else if this was a successful INVITE the ACK should:
                //     only come to this proxy if the INVITE was record-routed
                //  or the previous hop that send this ACK incorrectly sent the
                //     ACK to the same URI as the INVITE
                else
                {
                    shouldDispatch = TRUE;
                }
            }

            else
            {
                shouldDispatch = TRUE;
            }

        }
    } // End: Requests, provisional responses, Cancel response

#   ifdef TEST_PRINT
    osPrintf("SipTransaction::handleIncoming %p asking parent shouldDispatch=%d delayed=%p\n",
             this, shouldDispatch, delayedDispatchedMessage );
#   endif

#   ifdef DISPATCH_DEBUG
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming %p " 
                  "before handleChildIncoming shouldDispatch=%d delayed=%p\n",
                  this, shouldDispatch, delayedDispatchedMessage );
#   endif
    
    shouldDispatch =
        handleChildIncoming(incomingMessage,
                             userAgent,
                             relationship,
                             transactionList,
                             shouldDispatch,
                             delayedDispatchedMessage,
                             mpTransport);

#   ifdef DISPATCH_DEBUG
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming %p " 
                  "after handleChildIncoming shouldDispatch=%d delayed=%p\n",
                  this, shouldDispatch, delayedDispatchedMessage );
#   endif
#   ifdef TEST_PRINT
    osPrintf("SipTransaction::handleIncoming %p "
             "parent says shouldDispatch=%d delayed=%p\n",
             this, shouldDispatch, delayedDispatchedMessage );
#   endif
    touch();

    return(shouldDispatch);
} // end handleIncoming

void SipTransaction::removeTimer(OsTimer* timer)
{
    OsTimer* pt = static_cast<OsTimer*>(mTimers.find(timer));

    if(pt == NULL)
    {
#ifdef TEST_PRINT
        osPrintf("SipTransaction::removeTimer could not find timer %p in timer list.\n", timer);
#endif
    }
    else
    {
       OsTimer* removedTimer = static_cast<OsTimer*>(mTimers.remove(pt));

#ifdef TEST_PRINT
       if(removedTimer == NULL)
       {
           osPrintf("SipTransaction::removeTimer failed to remove timer %p from timer list.\n", timer);
       }
       else
       {
           osPrintf("SipTransaction::removeTimer removed timer %p from timer list.\n", timer);
       }
#else
       removedTimer = removedTimer; // supress unused variable warning
#endif
    }
}

void SipTransaction::deleteTimers()
{
    UtlSListIterator iterator(mTimers);
    OsTimer* timer = NULL;

    while ((timer = (OsTimer*)iterator()))
    {
#ifdef TEST_PRINT
        osPrintf("SipTransaction::deleteTimers deleting timer %p\n", timer);
#endif
        // stop timer so it doesn't fire anymore
        timer->stop();

        // remove timer from transaction
        removeTimer(timer);
        
        // delete timer if it's stopped and isn't stopped because it fired
        // but because we stopped it manually. Such timers are safe to delete.
        // Fired timers should be cleaned up in the message queue they posted
        // message to (SipUserAgent).
        if (timer->getState() == OsTimer::STOPPED && timer->getWasFired() == FALSE)
        {
            OsQueuedEvent *pEvent = (OsQueuedEvent*)timer->getNotifier();
            intptr_t userData;
            pEvent->getUserData(userData);
            SipMessageEvent* pMsgEvent = (SipMessageEvent*) userData;
            delete pMsgEvent;
            delete timer;
        }
    }
}

void SipTransaction::stopTimers()
{
    UtlSListIterator iterator(mTimers);
    OsTimer* timer = NULL;

    while ((timer = (OsTimer*)iterator()))
    {
        timer->stop();
    }
}

void SipTransaction::startTimers()
{
/*
    // As if 2006-10-06 -- The timer subsystem has been rewritten and no 
    // longer supports the ability to restart timers -- work is needed
    // to add this (not sure how much), but until then, disabling this
    // functionality.
    UtlSListIterator iterator(mTimers);
    OsTimer* timer = NULL;

    while ((timer = (OsTimer*)iterator()))
    {
        timer->start();
    }
*/
}

void SipTransaction::cancel(SipUserAgent& userAgent,
                            SipTransactionList& transactionList)
{
    if(mIsServerTransaction)
    {
        // SHould not get here this is only for kids (i.e. child client transactions)
        OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::cancel called on server transaction");
    }

    else if(!mIsCanceled)
    {
        mIsCanceled = TRUE;

        if(mpRequest)
        {
            if(mpCancel)
            {
                OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::cancel cancel request already exists");
            }
            // Do not send CANCELs for non-INVITE transactions
            // (all the other state stuff should be done)
            else if(mTransactionState == TRANSACTION_PROCEEDING &&
                    mIsDnsSrvChild &&
                    mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                // We can only send a CANCEL if we have heard
                // back a provisional response.  If we have a
                // final response it is too late
                SipMessage cancel;
                cancel.setCancelData(mpRequest);
#ifdef TEST_PRINT
                osPrintf("SipTransaction::cancel sending CANCEL\n");
#endif
                handleOutgoing(cancel,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL,
                               mpTransport);
            }

            //if(mIsRecursing)
            {
                cancelChildren(userAgent,
                               transactionList);
            }
        }

        // If this transaction has been initiated (i.e. request
        // has been created and sent)
        else if(mTransactionState != TRANSACTION_LOCALLY_INIITATED)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::cancel no request");
        }

    }

#ifdef TEST_PRINT
    else
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::cancel already canceled");
    }
#endif
}

void SipTransaction::cancelChildren(SipUserAgent& userAgent,
                                    SipTransactionList& transactionList)
{
    // Cancel all the child transactions
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        childTransaction->cancel(userAgent,
                                 transactionList);
    }
}

void SipTransaction::linkChild(SipTransaction& newChild)
{
    if(newChild.mpParentTransaction)
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::linkChild child.parent is not NULL");
    }
    newChild.mpParentTransaction = this;
    newChild.mIsBusy = mIsBusy;

    if(mChildTransactions.containsReference(&newChild))
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::linkChild child already a child");
    }
    else
    {
        // The children are supposed to be sorted by Q value,
        // largest first
        UtlSListIterator iterator(mChildTransactions);
        SipTransaction* childTransaction = NULL;
        UtlBoolean childInserted = FALSE;
        int sortIndex = 0;

        while ((childTransaction = (SipTransaction*) iterator()))
        {
            if(childTransaction->mQvalue < newChild.mQvalue)
            {
                mChildTransactions.insertAt(sortIndex, &newChild);
                childInserted = TRUE;
                break;
            }
            sortIndex++;
        }

        // It goes last
        if(!childInserted) mChildTransactions.append(&newChild);
    }

    if(mIsServerTransaction &&
        mIsUaTransaction)
    {
        mIsUaTransaction = FALSE;
        OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::linkChild converting server UA transaction to server proxy transaction");
    }
}

void SipTransaction::dumpTransactionTree(UtlString& dumpstring,
                                         UtlBoolean dumpMessagesAlso)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent)
    {
        parent->toString(dumpstring, dumpMessagesAlso);
        parent->dumpChildren(dumpstring, dumpMessagesAlso);
    }
}

void SipTransaction::dumpChildren(UtlString& dumpstring,
                                  UtlBoolean dumpMessagesAlso)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlString childString;

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // Dump Child
        childString.remove(0);
        childTransaction->toString(childString, dumpMessagesAlso);
        dumpstring.append(childString);

        // Dump childred recursively
        childString.remove(0);
        childTransaction->dumpChildren(childString, dumpMessagesAlso);
        dumpstring.append(childString);
    }
}

void SipTransaction::toString(UtlString& dumpString,
                              UtlBoolean dumpMessagesAlso)
{
    char numBuffer[64];

    dumpString.append("  SipTransaction dump:\n\tthis: ");
    sprintf(numBuffer, "%p", this);
    dumpString.append(numBuffer);

    dumpString.append("\n\thash: ");
    dumpString.append(this->data());
    dumpString.append("\n\tmCallId: ");
    dumpString.append(mCallId);
    dumpString.append("\n\tmBranchId: ");
    dumpString.append(mBranchId);
    dumpString.append("\n\tmRequestUri: ");
    dumpString.append(mRequestUri);
    dumpString.append("\n\tmSendToAddress: ");
    dumpString.append(mSendToAddress);
    dumpString.append("\n\tmSendToPort: ");
    sprintf(numBuffer, "%d", mSendToPort);
    dumpString.append(numBuffer);
    dumpString.append("\n\tmSendToProtocol: ");
    UtlString protocolString;
    SipMessage::convertProtocolEnumToString(mSendToProtocol,
        protocolString);
    dumpString.append(protocolString);
    //sprintf(numBuffer, "%d", mSendToProtocol);
    //dumpString.append(numBuffer);

    if(mpDnsSrvRecords)
    {
        dumpString.append("\n\tmpDnsSrvRecords:\n\t\tPref\tWt\tType\tName(ip):Port");
        UtlString srvName;
        UtlString srvIp;
        char srvRecordNums[128];
        for (int i=0; mpDnsSrvRecords[i].isValidServerT(); i++)
        {
            mpDnsSrvRecords[i].getHostNameFromServerT(srvName);
            mpDnsSrvRecords[i].getIpAddressFromServerT(srvIp);
            sprintf(srvRecordNums, "\n\t\t%d\t%d\t%d\t",
                mpDnsSrvRecords[i].getPriorityFromServerT(),
                mpDnsSrvRecords[i].getWeightFromServerT(),
                mpDnsSrvRecords[i].getProtocolFromServerT());
            dumpString.append(srvRecordNums);
            dumpString.append(srvName);
            dumpString.append("(");
            dumpString.append(srvIp);
            sprintf(srvRecordNums, "):%d",
                mpDnsSrvRecords[i].getPortFromServerT());
            dumpString.append(srvRecordNums);
        }
    }
    else
    {
        dumpString.append("\n\tmpDnsSrvRecords: NULL");
    }

    dumpString.append("\n\tmFromField: ");
    dumpString.append(mFromField.toString());
    dumpString.append("\n\tmToField: ");
    dumpString.append(mToField.toString());
    dumpString.append("\n\tmRequestMethod: ");
    dumpString.append(mRequestMethod);
    dumpString.append("\n\tmCseq: ");
    sprintf(numBuffer, "%d", mCseq);
    dumpString.append(numBuffer);
    dumpString.append("\n\tmIsServerTransaction: ");
    dumpString.append(mIsServerTransaction ? "TRUE" : " FALSE");
    dumpString.append("\n\tmIsUaTransaction: ");
    dumpString.append(mIsUaTransaction ? "TRUE" : " FALSE");

    UtlString msgString;
    int len;

    dumpString.append("\n\tmpRequest: ");
    if(mpRequest && dumpMessagesAlso)
    {
        mpRequest->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpRequest);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpLastProvisionalResponse: ");
    if(mpLastProvisionalResponse && dumpMessagesAlso)
    {
        mpLastProvisionalResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpLastProvisionalResponse)
        {
            sprintf(numBuffer, "%d ", mpLastProvisionalResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpLastProvisionalResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpLastFinalResponse: ");
    if(mpLastFinalResponse && dumpMessagesAlso)
    {
        mpLastFinalResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpLastFinalResponse)
        {
            sprintf(numBuffer, "%d ", mpLastFinalResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpLastFinalResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpAck: ");
    if(mpAck && dumpMessagesAlso)
    {
        mpAck->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpAck);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpCancel: ");
    if(mpCancel && dumpMessagesAlso)
    {
        mpCancel->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpCancel);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpCancelResponse: ");
    if(mpCancelResponse && dumpMessagesAlso)
    {
        mpCancelResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpCancelResponse)
        {
            sprintf(numBuffer, "%d", mpCancelResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpCancelResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpParentTransaction: ");
    sprintf(numBuffer, "%p", mpParentTransaction);
    dumpString.append(numBuffer);

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    int childCount = 0;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        dumpString.append("\n\tmChildTransactions[");
        sprintf(numBuffer, "%d]: %p", childCount, childTransaction);
        dumpString.append(numBuffer);
        childCount++;
    }
    if(childCount == 0)
    {
        dumpString.append("\n\tmChildTransactions: none");
    }

    dumpString.append("\n\tmTransactionCreateTime: ");
    sprintf(numBuffer, "%ld", mTransactionCreateTime);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmTransactionStartTime: ");
    sprintf(numBuffer, "%ld", mTransactionStartTime);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmTimeStamp: ");
    sprintf(numBuffer, "%ld", mTimeStamp);
    dumpString.append(numBuffer);

    UtlString state;
    getStateString(mTransactionState, state);
    dumpString.append("\n\tmTransactionState: ");
    dumpString.append(state);

    dumpString.append("\n\tmIsCanceled: ");
    dumpString.append(mIsCanceled ? "TRUE" : " FALSE");

    dumpString.append("\n\tmIsRecursing: ");
    dumpString.append(mIsRecursing ? "TRUE" : " FALSE");

    dumpString.append("\n\tmIsDnsSrvChild: ");
    dumpString.append(mIsDnsSrvChild ? "TRUE" : " FALSE");

    dumpString.append("\n\tmProvisionalSdp: ");
    dumpString.append(mProvisionalSdp ? "TRUE" : " FALSE");

    dumpString.append("\n\tmQvalue: ");
    sprintf(numBuffer, "%lf", mQvalue);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmExpires: ");
    sprintf(numBuffer, "%d", mExpires);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmIsBusy: ");
    sprintf(numBuffer, "%d", mIsBusy);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmBusyTaskName: ");
    dumpString.append(mBusyTaskName);

    dumpString.append("\n\tmWaitingList: ");
    sprintf(numBuffer, "%p ", mWaitingList);
    dumpString.append(numBuffer);
    if(mWaitingList)
    {
        sprintf(numBuffer, "(%" PRIuPTR ")", mWaitingList->entries());
        dumpString.append(numBuffer);
    }

    dumpString.append("\n");
}


void SipTransaction::notifyWhenAvailable(OsEvent* availableEvent)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && availableEvent)
    {
        if(parent->mWaitingList == NULL)
        {
            parent->mWaitingList = new UtlSList();
        }

        UtlSList* list = parent->mWaitingList;

        UtlVoidPtr* eventNode = new UtlVoidPtr((void*)availableEvent);

        list->append(eventNode);
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::notifyWhenAvailable parent: %p avialableEvent: %p",
            parent, availableEvent);
    }
}

void SipTransaction::signalNextAvailable()
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && parent->mWaitingList)
    {
        // Remove the first event that is waiting for this transaction
        UtlVoidPtr* eventNode = (UtlVoidPtr*) parent->mWaitingList->get();

        if(eventNode)
        {
            OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
//#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::signalNextAvailable %p signaling: %p",
                    parent, waitingEvent);
//#endif
            if(waitingEvent)
            {
                // If the event is already signaled, the other side
                // gave up waiting, so this side needs to free up
                // the event.
                if(waitingEvent->signal(1) == OS_ALREADY_SIGNALED)
                {
                    delete waitingEvent;
                    waitingEvent = NULL;
                }
            }
            delete eventNode;
            eventNode = NULL;
        }
    }
}

void SipTransaction::signalAllAvailable()
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && parent->mWaitingList)
    {
        UtlSList* list = parent->mWaitingList;
        // Remove the first event that is waiting for this transaction
        UtlVoidPtr* eventNode = NULL;
        while ((eventNode = (UtlVoidPtr*) list->get()))
        {
            if(eventNode)
            {
                OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::signalAllAvailable %p signaling: %p",
                    parent, waitingEvent);
#endif
                if(waitingEvent)
                {
                    // If the event is already signaled, the other side
                    // gave up waiting, so this side needs to free up
                    // the event.
                    if(waitingEvent->signal(1) == OS_ALREADY_SIGNALED)
                    {
                        delete waitingEvent;
                        waitingEvent = NULL;
                    }
                }
                delete eventNode;
                eventNode = NULL;
            }
        }
    }
}


/* ============================ ACCESSORS ================================= */

void SipTransaction::getStateString(enum transactionStates state,
                                           UtlString& stateString)
{
    switch(state)
    {
    case TRANSACTION_UNKNOWN:
        stateString = "TRANSACTION_UNKNOWN";
        break;

    case TRANSACTION_LOCALLY_INIITATED:
        stateString = "TRANSACTION_LOCALLY_INITIATED";
        break;

    case TRANSACTION_CALLING:
        stateString = "TRANSACTION_CALLING";
        break;

    case TRANSACTION_PROCEEDING:
        stateString = "TRANSACTION_PROCEEDING";
        break;

    case TRANSACTION_COMPLETE:
        stateString = "TRANSACTION_COMPLETE";
        break;

    case TRANSACTION_CONFIRMED:
        stateString = "TRANSACTION_CONFIRMED";
        break;

    case TRANSACTION_TERMINATED:
        stateString = "TRANSACTION_TERMINATED";
        break;

    default:
        char numBuffer[128];
        sprintf(numBuffer, "UKNOWN Transaction state: %d", state);
        stateString = numBuffer;
        break;
    }
}

void SipTransaction::getRelationshipString(enum messageRelationship relationship,
                                           UtlString& relationshipString)
{
    switch(relationship)
    {
    case MESSAGE_UNKNOWN:
        relationshipString = "MESSAGE_UNKNOWN";
        break;

        case MESSAGE_UNRELATED:
        relationshipString = "MESSAGE_UNRELATED";
        break;

        case MESSAGE_SAME_SESSION:
        relationshipString = "MESSAGE_SAME_SESSION";
        break;

        case MESSAGE_DIFFERENT_BRANCH:
        relationshipString = "MESSAGE_DIFFERENT_BRANCH";
        break;

        case MESSAGE_REQUEST:
        relationshipString = "MESSAGE_REQUEST";
        break;

        case MESSAGE_PROVISIONAL:
        relationshipString = "MESSAGE_PROVISIONAL";
        break;

        case MESSAGE_FINAL:
        relationshipString = "MESSAGE_FINAL";
        break;

        case MESSAGE_NEW_FINAL:
        relationshipString = "MESSAGE_NEW_FINAL";
        break;

        case MESSAGE_CANCEL:
        relationshipString = "MESSAGE_CANCEL";
        break;

        case MESSAGE_ACK:
        relationshipString = "MESSAGE_ACK";
        break;

        case MESSAGE_2XX_ACK:
        relationshipString = "MESSAGE_2XX_ACK";
        break;

        case MESSAGE_DUPLICATE:
        relationshipString = "MESSAGE_DUPLICATE";
        break;

        default:
        char buffer[128];
        sprintf(buffer, "UKNOWN transaction relationship: %d",
            relationship);
        relationshipString = buffer;
        break;
    }
}

void SipTransaction::buildHash(const SipMessage& message,
                              UtlBoolean isOutgoing,
                              UtlString& hash)
{
    UtlBoolean isServerTransaction =
        message.isServerTransaction(isOutgoing);

    message.getCallIdField(&hash);
    hash.append(isServerTransaction ? 's' : 'c');

    int cSeq;
    //UtlString method;
    message.getCSeqField(&cSeq, NULL /*&method*/);
    char cSeqString[20];
    sprintf(cSeqString, "%d", cSeq);
    hash.append(cSeqString);
}

SipTransaction* SipTransaction::getTopMostParent() const
{
    SipTransaction* topParent = NULL;
    if(mpParentTransaction)
    {
        topParent = mpParentTransaction->getTopMostParent();

        if(topParent == NULL)
        {
            topParent = mpParentTransaction;
        }
    }

    return(topParent);
}

void SipTransaction::getCallId(UtlString& callId) const
{
    callId = mCallId;
}

enum SipTransaction::transactionStates SipTransaction::getState() const
{
    return(mTransactionState);
}

/*long SipTransaction::getStartTime() const
{
    return(mTransactionStartTime);
}*/

long SipTransaction::getTimeStamp() const
{
    return(mTimeStamp);
}

void SipTransaction::touch()
{
    // We touch the whole parent, child tree so that
    // none of transactions get garbage collected
    // until they are all stale.  This saves checking
    // up and down the tree during garbage collection
    // to see if there are any still active transactions.

    SipTransaction* topParent = getTopMostParent();

    // We end up setting the date twice on this
    // transaction if it is not the top most parent
    // but so what.  The alternative is using a local
    // variable to hold the date.  There is no net
    // savings.
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    mTimeStamp = time.seconds();
    //osPrintf("SipTransaction::touch seconds: %ld usecs: %ld\n",
    //    time.seconds(), time.usecs());
    //mTimeStamp = OsDateTime::getSecsSinceEpoch();

    if(topParent)
    {
        topParent->touchBelow(mTimeStamp);
    }
    else
    {
        touchBelow(mTimeStamp);
    }
}

void SipTransaction::touchBelow(int newDate)
{
    mTimeStamp = newDate;

#ifdef TEST_PRINT
    UtlString serialized;
    toString(serialized, FALSE);
    osPrintf("%s\n", serialized.data());
#endif // TEST_PRINT

    SipTransaction* child = NULL;
    UtlSListIterator iterator(mChildTransactions);
    while((child = (SipTransaction*) iterator()))
    {
        child->touchBelow(newDate);
    }
}


SipMessage* SipTransaction::getRequest()
{
    return(mpRequest);
}

SipMessage* SipTransaction::getLastProvisionalResponse()
{
    return(mpLastProvisionalResponse);
}

SipMessage* SipTransaction::getLastFinalResponse()
{
    return(mpLastFinalResponse);
}

void SipTransaction::markBusy()
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::markBusy %p", this);
#endif
    if(mpParentTransaction) mpParentTransaction->markBusy();
    else
    {
        OsTime time;
        OsDateTime::getCurTimeSinceBoot(time);
        int busyTime = time.seconds();
        // Make sure it is not equal to zero
        if(!busyTime) busyTime++;
        doMarkBusy(busyTime);

        OsTask* busyTask = OsTask::getCurrentTask();
        if(busyTask) mBusyTaskName = busyTask->getName();
        else mBusyTaskName = "";
    }
}

void SipTransaction::doMarkBusy(int markData)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::doMarkBusy(%d) %p", markData, this);
#endif

    mIsBusy = markData;

    // Recurse through the children and mark them busy
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        childTransaction->doMarkBusy(markData);
    }
}

void SipTransaction::markAvailable()
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransaction::markAvailable %p", this);
#endif

    // Recurse to the parent
    if(mpParentTransaction) mpParentTransaction->markAvailable();

    // This is the top most parent
    else
    {
        touch(); //set the last used time for parent and all children
        doMarkBusy(0);
        signalNextAvailable();
    }
}

/*void SipTransaction::doMarkAvailable()
{
    mIsBusy = FALSE;

    // Recurse through the children and mark them available
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while(childTransaction = (SipTransaction*) iterator())
    {
        childTransaction->doMarkAvailable();
    }
}
*/

/* ============================ INQUIRY =================================== */

UtlBoolean SipTransaction::isServerTransaction() const
{
    return(mIsServerTransaction);
}

UtlBoolean SipTransaction::isDnsSrvChild() const
{
    return(mIsDnsSrvChild);
}

UtlBoolean SipTransaction::isUaTransaction() const
{
    return(mIsUaTransaction);
}

UtlBoolean SipTransaction::isChildSerial()
{
    // The child transactions are supposed to be sorted by
    // Q value.  So if we look at the first and last and they
    // are different then there are serially searched children

    UtlBoolean isSerial = FALSE;
    SipTransaction* child = (SipTransaction*)mChildTransactions.first();
    if(child)
    {
        double q1 = child->mQvalue;

        child = (SipTransaction*)mChildTransactions.last();
        if(child)
        {
            double q2 = child->mQvalue;
            if((q1-q2)*(q1-q2) > MIN_Q_DELTA_SQUARE)
            {
                isSerial = TRUE;
            }
        }

    }

    return(isSerial);
}

UtlBoolean SipTransaction::isEarlyDialogWithMedia()
{
    UtlBoolean earlyDialogWithMedia = FALSE;

    if(mProvisionalSdp &&
       mTransactionState > TRANSACTION_LOCALLY_INIITATED &&
       mTransactionState < TRANSACTION_COMPLETE)
    {
        earlyDialogWithMedia = TRUE;

        // This should not occur, the state should be ?TERMINATED?
        if(mIsCanceled)
        {
            UtlString stateString;
            SipTransaction::getStateString(mTransactionState, stateString);

            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipTransaction::isEarlyDialogWithMedia transaction state: %s incorrect for canceled transaction",
                stateString.data());
        }

        // This should not occur, the state should be COMPETED or CONFIRMED
        if(mIsRecursing)
        {
            UtlString stateString;
            SipTransaction::getStateString(mTransactionState, stateString);

            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipTransaction::isEarlyDialogWithMedia transaction state: %s incorrect for recursing transaction",
                stateString.data());
        }
    }

    return(earlyDialogWithMedia);
}

UtlBoolean SipTransaction::isChildEarlyDialogWithMedia()
{
    UtlBoolean earlyDialogWithMedia = FALSE;
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // If the state is initiated, no request has been sent
        // for this transaction and this an all other children after
        // this one in the list should be in the same state
        if(childTransaction->mTransactionState ==
            TRANSACTION_LOCALLY_INIITATED)
        {
            break;
        }

        earlyDialogWithMedia = childTransaction->isEarlyDialogWithMedia();
    }

    return(earlyDialogWithMedia);
}

UtlBoolean SipTransaction::isMethod(const char* methodToMatch) const
{
    return(strcmp(mRequestMethod.data(), methodToMatch) == 0);
}

enum SipTransaction::messageRelationship
SipTransaction::whatRelation(const SipMessage& message,
                             UtlBoolean isOutgoing) const
{
    enum messageRelationship relationship = MESSAGE_UNKNOWN;
    UtlString msgCallId;
    message.getCallIdField(&msgCallId);

    // Note: this is nested to bail out as soon as possible
    // for efficiency reasons

    // CallId matches
    if(mCallId.compareTo(msgCallId) == 0)
    {
        int msgCseq;
        UtlString msgMethod;
        message.getCSeqField(&msgCseq, &msgMethod);
        UtlBoolean isResponse = message.isResponse();
        int lastFinalResponseCode = mpLastFinalResponse ?
            mpLastFinalResponse->getResponseStatusCode() : -1;

        UtlString viaField;
        UtlString msgBranch;
        UtlBoolean msgHasVia = message.getViaFieldSubField(&viaField, 0);
        SipMessage::getViaTag(viaField.data(), "branch",
           msgBranch);

        //osPrintf("SipTransaction::whatRelation\n\tvia: %s \tbranch: %s\n",
        //    viaField.data(), msgBranch.data());

        UtlBoolean branchPrefixSet =
            (mBranchId.index(BRANCH_ID_PREFIX) != UTL_NOT_FOUND);

        UtlBoolean toTagMatches = FALSE;
        UtlBoolean fromTagMatches;
        UtlBoolean branchIdMatches = mBranchId.compareTo(msgBranch) == 0;
        UtlBoolean mustCheckTags;
        bool bCustomTransport;
        message.getTransportName(bCustomTransport);

        // If the branch prefix is set, we can assume that
        // all we need to look at is the branch parameter to
        // determine if this is the same transaction or not.
        // ACK to INVITEs with 200 response are really a different
        // transaction.  For convenience we consider them to be
        // the same.  CANCLE is also a different transaction
        // that we store in the same SipTransaction object.
        if(bCustomTransport || !branchPrefixSet ||
           (!isResponse &&
               (msgMethod.compareTo(SIP_CANCEL_METHOD) == 0 ||
               (msgMethod.compareTo(SIP_ACK_METHOD) == 0 &&
                lastFinalResponseCode < SIP_3XX_CLASS_CODE &&
                lastFinalResponseCode >= SIP_2XX_CLASS_CODE) ||
                (!mIsServerTransaction &&
                mTransactionState == TRANSACTION_LOCALLY_INIITATED))))
        {
            // Avoid looking at the tags as it is expensive to
            // parse the To and From fields into a Url object.
            mustCheckTags = TRUE;

            Url msgFrom;
            UtlString msgFromTag;
            UtlString fromTag;
            message.getFromUrl(msgFrom);
            msgFrom.getFieldParameter("tag", msgFromTag);

            Url *pFromUrl = (Url *)&mFromField;
            pFromUrl->getFieldParameter("tag", fromTag);
            fromTagMatches = msgFromTag.compareTo(fromTag) == 0;
        }
        else
        {
            mustCheckTags = FALSE;
            toTagMatches  = branchIdMatches;
            fromTagMatches = branchIdMatches;
        }


        // From field tag matches
        if(fromTagMatches)
        {
            UtlString msgToTag;
            // Do not check if the to tag matches untill we know
            // the from tag matches.  Also do not look at the to
            // tag if we can determine a match from the branch.
            // The parsing the To field into a Url is expensive
            // so we avoid it if we can.
            if(mustCheckTags)
            {
                Url msgTo;

                UtlString toTag;
                message.getToUrl(msgTo);
                msgTo.getFieldParameter("tag", msgToTag);

                Url *pToUrl = (Url *)&mToField;
                pToUrl->getFieldParameter("tag", toTag);

                toTagMatches = (toTag.isNull() ||
                    toTag.compareTo(msgToTag) == 0);
            }

            // To field tag matches
            if(toTagMatches)
            {
                if(mCseq == msgCseq)
                {
                    UtlBoolean isMsgServerTransaction =
                        message.isServerTransaction(isOutgoing);
                    // The message and this transaction are either both
                    // part of a server or client transaction
                    if(isMsgServerTransaction == mIsServerTransaction)
                    {
                        UtlString finalResponseToTag;
                        // Note getting and parsing a URL is expensive so
                        // we avoid it
                        if(mpLastFinalResponse &&
                           mustCheckTags)
                        {
                            Url responseTo;
                            mpLastFinalResponse->getToUrl(responseTo);
                            responseTo.getFieldParameter("tag", finalResponseToTag);
                        }

                        UtlString msgUri;
                        UtlBoolean parentBranchIdMatches = FALSE;
                        if(!isResponse && // request
                           !mIsServerTransaction && // client transaction
                            mTransactionState == TRANSACTION_LOCALLY_INIITATED)
                        {
                            SipTransaction* parent = getTopMostParent();
                            // Should this matter whether it is a server or client TX?
                            if(parent &&
                                parent->mIsServerTransaction &&
                                msgBranch.compareTo(parent->mBranchId) == 0)
                            {
                                // We know that the request originated from
                                // this client transaction's parent server
                                // transaction.
                                parentBranchIdMatches = TRUE;
                                message.getRequestUri(&msgUri);
                            }
                            else if ((parent == NULL) && !msgHasVia)
                            {
                                // This is a client transaction with no parent and
                                // it originated from this UA because there are no
                                // vias
                                parentBranchIdMatches = TRUE;
                                message.getRequestUri(&msgUri);
                            }
                        }

                        // The branch of this message matches the
                        // transaction or
                        // This is the request for this client transaction
                        // and the Via had not been added yet for this
                        // transaction.  So the branch is not there yet.
                        if(branchIdMatches ||
                           (parentBranchIdMatches &&
                            msgUri.compareTo(mRequestUri) == 0) ||
                             (mIsUaTransaction && // UA client ACK transaction for 2xx
                             !mIsServerTransaction &&
                             !isResponse &&
                             !msgHasVia &&
                             msgMethod.compareTo(SIP_ACK_METHOD) == 0 &&
                             lastFinalResponseCode < SIP_3XX_CLASS_CODE &&
                             lastFinalResponseCode >= SIP_2XX_CLASS_CODE) ||
                             (mIsUaTransaction && // UA server ACK transaction for 2xx
                             mIsServerTransaction &&
                             !isResponse &&
                             msgMethod.compareTo(SIP_ACK_METHOD) == 0 &&
                             lastFinalResponseCode < SIP_3XX_CLASS_CODE &&
                             lastFinalResponseCode >= SIP_2XX_CLASS_CODE &&
                             finalResponseToTag.compareTo(msgToTag) == 0) ||
                             (mIsUaTransaction && // UA client CANCEL transaction
                             !mIsServerTransaction &&
                             !isResponse &&
                             !msgHasVia &&
                             msgMethod.compareTo(SIP_CANCEL_METHOD) == 0))
                        {
                            if(isResponse)
                            {
                                int msgResponseCode =
                                    message.getResponseStatusCode();

                                // Provisional responses
                                if(msgResponseCode < SIP_2XX_CLASS_CODE)
                                {
                                    relationship = MESSAGE_PROVISIONAL;
                                }

                                // Final responses
                                else
                                {
                                    if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransaction::messageRelationship ACK response");
                                    }

                                    else if(msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        relationship = MESSAGE_CANCEL_RESPONSE;
                                    }

                                    else if(mpLastFinalResponse)
                                    {
                                        int finalResponseCode =
                                            mpLastFinalResponse->getResponseStatusCode();
                                        if(finalResponseCode == msgResponseCode)
                                        {
                                            if (msgMethod.compareTo(SIP_INVITE_METHOD) == 0)
                                            {
                                                if(!mustCheckTags)
                                                {
                                                    Url msgTo;

                                                    message.getToUrl(msgTo);
                                                    msgTo.getFieldParameter("tag", msgToTag);

                                                    UtlString toTag;
                                                    Url toUrl;
                                                    mpLastFinalResponse->getToUrl(toUrl);
                                                    toUrl.getFieldParameter("tag", toTag);

                                                    toTagMatches = (toTag.isNull() ||
                                                        toTag.compareTo(msgToTag) == 0);
                                                }

                                                if (toTagMatches)
                                                {
#ifdef TEST_PRINT
                                                    osPrintf("DUPLICATED msg finalResponseCode - %d toTagMatches %d \n", finalResponseCode, toTagMatches);
#endif
                                                    relationship = MESSAGE_DUPLICATE;
                                                }
                                                else
                                                {
#ifdef TEST_PRINT
                                                    osPrintf("MESSAGE_NEW_FINAL - finalResponseCode - %d toTagMatches %d \n", finalResponseCode, toTagMatches);
#endif
                                                    relationship = MESSAGE_NEW_FINAL;
                                                }

                                            }
                                            else
                                            {
#ifdef TEST_PRINT
                                               osPrintf("DUPLICATED msg finalResponseCode - %d \n", finalResponseCode);
#endif
                                               relationship = MESSAGE_DUPLICATE;
                                            }
                                        }
                                        else
                                        {
                                            relationship = MESSAGE_NEW_FINAL;
                                        }
                                    }
                                    else
                                    {
                                        relationship = MESSAGE_FINAL;
                                    }
                                }
                            }

                            // Requests
                            else
                            {
                                if(mpRequest)
                                {
                                    UtlString previousRequestMethod;
                                    mpRequest->getRequestMethod(&previousRequestMethod);

                                    if(previousRequestMethod.compareTo(msgMethod) == 0)
                                    {
#ifdef TEST_PRINT
                                        osPrintf("DUPLICATED msg previousRequestMethod - %s \n", previousRequestMethod.data());
#endif
                                        relationship = MESSAGE_DUPLICATE;
                                    }
                                    else if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        if(mpLastFinalResponse)
                                        {
                                            int finalResponseCode =
                                                mpLastFinalResponse->getResponseStatusCode();
                                            if(finalResponseCode >= SIP_3XX_CLASS_CODE)
                                            {
                                                relationship = MESSAGE_ACK;
                                            }
                                            else
                                            {
                                                if(!mIsUaTransaction)
                                                {
                                                    OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::messageRelationship ACK matches transaction with 2XX class response");
                                                }
                                                relationship = MESSAGE_2XX_ACK;
                                            }
                                        }
                                        else
                                        {
                                            OsSysLog::add(FAC_SIP, PRI_WARNING, "SipTransaction::messageRelationship ACK matches transaction with NO final response");
                                            relationship = MESSAGE_ACK;
                                        }
                                    }
                                    else if(msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        relationship = MESSAGE_CANCEL;
                                    }
                                    else
                                    {
                                        relationship = MESSAGE_DUPLICATE;
                                        OsSysLog::add(FAC_SIP, PRI_WARNING, "WARNING SipTransaction::messageRelationship found %s request for transaction with %s",
                                            msgMethod.data(), previousRequestMethod.data());

                                    }
                                }
                                else
                                {
                                    if(msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        relationship = MESSAGE_CANCEL;
                                    }
                                    else if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        relationship = MESSAGE_ACK;
                                    }
                                    else
                                    {
                                        relationship = MESSAGE_REQUEST;
                                    }
                                }
                            }
                        }
                        else
                        {
                            relationship = MESSAGE_DIFFERENT_BRANCH;
                        }
                    }
                    else
                    {
                        relationship = MESSAGE_DIFFERENT_BRANCH;
                    }
                }
                else
                {
                    relationship = MESSAGE_SAME_SESSION;
                }
            }
            else
            {
                relationship = MESSAGE_UNRELATED;
            }
        }
        else
        {
            relationship = MESSAGE_UNRELATED;
        }

    }
    else
    {
        relationship = MESSAGE_UNRELATED;
    }

    return(relationship);
}

UtlBoolean SipTransaction::isBusy()
{
    return(mIsBusy);
}

UtlBoolean SipTransaction::isUriChild(Url& uri)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean childHasSameUri = FALSE;
    UtlString uriString;
    uri.getUri(uriString);

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        if(uriString.compareTo(childTransaction->mRequestUri) == 0)
        {
            childHasSameUri = TRUE;
            break;
        }
    }

    return(childHasSameUri);
}

UtlBoolean SipTransaction::isUriRecursed(Url& uri)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;
    UtlString uriString;
    uri.getUri(uriString);

    return(isUriRecursedChildren(uriString));
}

UtlBoolean SipTransaction::isUriRecursedChildren(UtlString& uriString)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean childHasSameUri = FALSE;

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        if(childTransaction->mTransactionState > TRANSACTION_LOCALLY_INIITATED &&
            uriString.compareTo(childTransaction->mRequestUri) == 0)
        {
            childHasSameUri = TRUE;
            break;
        }

        // Check recursively
        if(childTransaction->mTransactionState > TRANSACTION_LOCALLY_INIITATED &&
            isUriRecursedChildren(uriString))
        {
            childHasSameUri = TRUE;
            break;
        }
    }

    return(childHasSameUri);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
