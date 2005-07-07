// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#include <stdlib.h>

#if defined(_VXWORKS)
    #include <taskLib.h>
    #include <netinet/in.h>
#endif

#include <utl/UtlHashBagIterator.h>

#include "os/OsStatus.h"
#include "os/OsConfigDb.h"
#include "os/OsEventMsg.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "net/SipRefreshMgr.h"
#include "net/SipMessageEvent.h"
#include "net/NameValueTokenizer.h"
#include "net/SipObserverCriteria.h"
#include "net/Url.h"
#include "net/SipUserAgent.h"
#include "utl/UtlTokenizer.h"

#define UNREGISTER_CSEQ_NUMBER   2146483648   // 2^31 - 1,000,000

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// #define TEST_PRINT 1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SipRefreshMgr::SipRefreshMgr():
    OsServerTask("SipRefreshMgr-%d"),
    mRegisterListMutexR(OsRWMutex::Q_FIFO),
    mRegisterListMutexW(OsRWMutex::Q_FIFO),
    mSubscribeListMutexR(OsRWMutex::Q_FIFO),
    mSubscribeListMutexW(OsRWMutex::Q_FIFO),
    mIsStarted(FALSE),
    mObserverMutex(OsRWMutex::Q_FIFO),
    mUAReadyMutex(OsRWMutex::Q_FIFO),
    mMyUserAgent(NULL)
{}

SipRefreshMgr::~SipRefreshMgr()
{}

/*===================================================================*/
//INITIALIZED
/*===================================================================*/
UtlBoolean
SipRefreshMgr::init(
    SipUserAgent *ptrToMyAgent,
    int sipTcpPort,
    int sipUdpPort,
    const char* defaultUser,
    const char* publicAddress,
    const char* defaultAddress,
    const char* sipDirectoryServers,
    const char* sipRegistryServers,
    int defaultRegistryTimeout,
    int defaultSubscribeTimeout,
    int restartCount,
    const char* macAddress )
{
    if ( ptrToMyAgent != NULL )
    {
        mMyUserAgent = ptrToMyAgent;

        //set default paremeters
        mTcpPort = sipTcpPort;
        mUdpPort = sipUdpPort;

        if ( defaultAddress )
            mDefaultSipAddress.append( defaultAddress );

        if ( publicAddress && *publicAddress )
            mSipIpAddress.append( publicAddress );
        else
            OsSocket::getHostIp( &mSipIpAddress );

        if ( sipRegistryServers )
            mRegistryServer.append(sipRegistryServers);

        if ( defaultUser && *defaultUser )
            mDefaultUser.append(defaultUser);

        if ( macAddress && *macAddress )
            mMacAddress.append(macAddress);

        // defaultRegistryTimeout is 1 hr
        if ( defaultRegistryTimeout <= 0 )
            mDefaultRegistryPeriod = 3600;
        else
            mDefaultRegistryPeriod = defaultRegistryTimeout;

        // defaultSubscribeTimeout is 24 hr
        if ( defaultSubscribeTimeout <= 0 )
            mDefaultSubscribePeriod = 60*60*24;
        else
            mDefaultSubscribePeriod = defaultSubscribeTimeout;

        mRestartCount = restartCount;
        char strCount[64];
        sprintf( strCount, "%d", mRestartCount );
        mRestartCountStr.append( strCount );
        return true;
    } else
    {
        osPrintf("ERROR:SipRefreshMgr::Init: NULL SipUserAgent\n");
        return false;
    }
}

void
SipRefreshMgr::StartRefreshMgr()
{
    if ( !isStarted() )
    {   // start the thread
        start();
    }
    OsMsg StartMsg( OsMsg::REFRESH_MSG, SipRefreshMgr::START_REFRESH_MGR );
    postMessage(StartMsg);
#ifdef TEST_PRINT
    osPrintf("SipRefreshMgr: End of constructor\n");
#endif
}

void
SipRefreshMgr::waitForUA()
{
#ifdef TEST_PRINT
    osPrintf("SipRefreshMgr::waitForUA - Start\n");
#endif
    OsLock lock( mUAReadyMutex );

    // wait for the UA to start and get the NAT address
    mMyUserAgent->waitUntilReady();


    // Bob 2/10/03: Ideally, we would only listen for requests and responses, 
    //     but not incoming messages.
    // 
    //     However, we don't seem to receive transport errors caused by no 
    //     udp/tcp user agent at the target (nobody home).  So, we need to 
    //     add an incoming messages listener.
    //
    //     NOTE: Not adding a response listener doesn't work either.  It seems
    //     that we miss some auth events.
    //
    mMyUserAgent->addMessageObserver(
        *(this->getMessageQueue()),
        SIP_REGISTER_METHOD,
        TRUE,       // want to get requests
        TRUE,       // want to get responses
        TRUE,       // Incoming messages
        FALSE);     // Don't want to see out going messages

    mMyUserAgent->addMessageObserver(
        *(this->getMessageQueue()),
        SIP_SUBSCRIBE_METHOD,
        TRUE,       // want to get requests
        TRUE,       // want to get responses
        TRUE,       // Incoming messages
        FALSE);     // Don't want to see out going messages

    // register all lines
    reRegisterAll();

    // subscribe to all subscriptions, MWI, CONFIG SERVER etc.
    reSubscribeAll();

    mIsStarted = TRUE;

#ifdef TEST_PRINT
    osPrintf("SipRefreshMgr::waitForUA - End\n");
#endif
}

UtlBoolean
SipRefreshMgr::isUAStarted()
{
    OsLock lock(mUAReadyMutex);
    return(mIsStarted);
}

void
SipRefreshMgr::addMessageConsumer( OsServerTask* messageEventListener )
{
    // Need to do the real thing by keeping a list of consumers
    // and putting a mutex around the add to list
    //if(messageListener)
    //{
    //  osPrintf("WARNING: message consumer is NOT a LIST\n");
    //}
    //messageListener = messageEventListener;
    if ( messageEventListener )
    {
        addMessageObserver( *(messageEventListener->getMessageQueue()));
    }
}

void
SipRefreshMgr::addMessageObserver (
    OsMsgQ& messageQueue,
    const char* sipMethod,
    UtlBoolean wantRequests,
    UtlBoolean wantResponses,
    UtlBoolean wantIncoming,
    UtlBoolean wantOutGoing,
    const char* eventName,
    void* observerData)
{
    SipObserverCriteria* observer =
        new SipObserverCriteria(
                observerData,
                &messageQueue,
                sipMethod,
                wantRequests,
                wantResponses,
                wantIncoming,
                wantOutGoing,
                eventName );
    {
        // Add the observer and its filter criteria to the list lock scope
        OsWriteLock lock(mObserverMutex);
        mMessageObservers.insert(observer);
    }
}

/*===================================================================*/
//REGISTER
/*===================================================================*/
UtlBoolean 
SipRefreshMgr::newRegisterMsg( 
    const Url& fromUrl,
    const UtlString& lineId,
    int registryPeriodSeconds )
{
    if ( !isDuplicateRegister( fromUrl ) )
    {
        syslog(FAC_REFRESH_MGR, PRI_DEBUG, "adding registration:\nurl=%s\nlineid=%s\nperiod=%d",
                fromUrl.toString().data(), lineId.data(), registryPeriodSeconds) ;
        
        Url uri = fromUrl;
        uri.setDisplayName("");
        uri.setUserId("");

        //generate Call Id
        UtlString registerCallId;
        generateCallId( 
            fromUrl.toString(),
            SIP_REGISTER_METHOD, 
            registerCallId );

        registerUrl(fromUrl.toString(), // from
                    fromUrl.toString(), // to
                    uri.toString(),
                    registerCallId,
                    lineId,
                    registryPeriodSeconds);

        return true;
    }
    else
    {
        syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to add new registration (dup):\nurl=%s\nlineid=%s\nperiod=%d",
                fromUrl.toString().data(), lineId.data(), registryPeriodSeconds) ;
    }

    return false;
}


/*---------------------------------------------------*/
void 
SipRefreshMgr::getFromAddress(
    UtlString* address, 
    int* port, 
    UtlString* protocol )
{
    UtlTokenizer tokenizer(mRegistryServer);
    UtlString regServer;

    tokenizer.next(regServer, ",");
    SipMessage::parseAddressFromUri(regServer.data(), address, port, protocol);

    if ( address->isNull() )
    {
        protocol->remove(0);
        // TCP only
        if ( mTcpPort > 0 && mUdpPort <= 0 )
        {
            protocol->append(SIP_TRANSPORT_TCP);
            *port = mTcpPort;
        }
        // UDP only
        else if ( mUdpPort > 0 && mTcpPort <= 0 )
        {
            protocol->append(SIP_TRANSPORT_UDP);
            *port = mUdpPort;
        }
        // TCP & UDP on non-standard port
        else if ( mTcpPort != SIP_PORT )
        {
            *port = mTcpPort;
        }
        // TCP & UDP on standard port
        else
        {
            *port = 0;
        }

        // If there is an address configured use it
        NameValueTokenizer::getSubField(mDefaultSipAddress.data(), 0, ", \t", address);
        // else use the local host ip address
        if ( address->isNull() )
        {
            address->append(mSipIpAddress);
        }
    }

}/*---------------------------------------------------*/
void SipRefreshMgr::reRegisterAll()
{
    SipMessageList tempList;
    SipMessage* listMessage;
    int iteratorHandle;

#ifdef TEST_PRINT
    osPrintf("SipRefreshMgr::reRegisterAll start \n");
#endif

    //scope the locks
    {
        OsReadLock readlock(mRegisterListMutexR);
        OsWriteLock writeLock(mRegisterListMutexW);

        iteratorHandle = mRegisterList.getIterator();
        while ((listMessage = (SipMessage*) mRegisterList.getSipMessageForIndex(iteratorHandle)))
        {
            //somehow stop the old timer - will get not cause harm because will not be able to find corresponding request.
            tempList.add(listMessage);
        }
        mRegisterList.releaseIterator(iteratorHandle);
    }

    iteratorHandle = tempList.getIterator();
    while ((listMessage = (SipMessage*) tempList.getSipMessageForIndex(iteratorHandle)))
    {
#ifdef TEST_PRINT
        osPrintf("SipRefreshMgr::reRegisterAll calling rescheduleRequest()\n");
#endif
        //sendRequest(*listMessage , SIP_REGISTER_METHOD);
        rescheduleRequest(listMessage, 1 , SIP_REGISTER_METHOD, DEFAULT_PERCENTAGE_TIMEOUT, TRUE);
    }
    tempList.releaseIterator(iteratorHandle);
}
/*---------------------------------------------------*/
void SipRefreshMgr::reRegister( const Url& fromUrl)
{
    SipMessage *oldMsg = mRegisterList.isSameFrom( fromUrl);
    if ( oldMsg )
    {
        SipMessage newMsg(*oldMsg);
        newMsg.incrementCSeqNumber();

        // Clear the DNS field, so that we retry DNS-SRV before resending.  
        // This should be performed for all failure cases, except for
        // auth challenges
        newMsg.clearDNSField() ;
        newMsg.resetTransport() ;

        addToRegisterList(&newMsg);
        UtlBoolean bRemovedOk = removeFromRegisterList(oldMsg);

        if (sendRequest(newMsg , SIP_REGISTER_METHOD) != OS_SUCCESS)
            removeFromRegisterList(&newMsg);
            
        if (bRemovedOk)
        {
            delete oldMsg;
            oldMsg = NULL;
        }
    }
}


/*---------------------------------------------------*/
void 
SipRefreshMgr::unRegisterUser (
    const Url& fromUrl, 
    const UtlBoolean& onStartup, 
    const UtlString& lineId )
{
    if ( onStartup )
    {

        Url Uri = fromUrl;
        Uri.setDisplayName("");
        Uri.setUserId("");

        //generate Call Id
        UtlString registerCallId;
        generateCallId(fromUrl.toString(),SIP_REGISTER_METHOD, registerCallId, TRUE);

        SipMessage* regMessage = new SipMessage();
        UtlString contactField;
        getContactField( fromUrl, contactField, lineId );

        UtlString fromField(fromUrl.toString());
        UtlString toField(fromField);
        //add Tag to from field
        UtlString tagNamevaluePair ;
        createTagNameValuePair(tagNamevaluePair);
        fromField.append(";");
        fromField.append(tagNamevaluePair);

        Url contact(contactField);
        contact.setFieldParameter(SIP_EXPIRES_FIELD,"0");

        regMessage->setRegisterData(fromField.data(), // from
                                    toField.data(), // to
                                    Uri.toString(), // uri
                                    contact.toString().data(), // contact
                                    registerCallId.data(),
                                    UNREGISTER_CSEQ_NUMBER,
                                    0);

        regMessage->removeHeader(SIP_EXPIRES_FIELD,0);
        if (sendRequest(*regMessage , SIP_REGISTER_METHOD) != OS_SUCCESS)
            removeFromRegisterList(regMessage);

    } else
    {
        SipMessage sipMsg;
        if ( isDuplicateRegister(fromUrl, sipMsg) )
        {
            //dont set a common expires - then you need to send * in contact field
            //sipMsg.setExpiresField(0);
            UtlString contactField;
            sipMsg.getContactField(0,contactField);
            Url contact(contactField);
            contact.setFieldParameter(SIP_EXPIRES_FIELD,"0");
            sipMsg.setContactField(contact.toString());
            sipMsg.removeHeader(SIP_EXPIRES_FIELD,0);

            rescheduleRequest(&sipMsg, 1 , SIP_REGISTER_METHOD , DEFAULT_PERCENTAGE_TIMEOUT,TRUE);
        }
    }
}

UtlBoolean 
SipRefreshMgr::isDuplicateRegister (
    const Url& fromUrl, 
    SipMessage &oldMsg )
{
    OsReadLock readlock(mRegisterListMutexR);
    OsWriteLock writeLock(mRegisterListMutexW);
    // call copy constructor on the oldMsg
    SipMessage* pduplicate = mRegisterList.isSameFrom(fromUrl);
    if ( pduplicate != NULL )
    {
        oldMsg = *pduplicate;
        return true;
    }
    return false;
}

UtlBoolean 
SipRefreshMgr::isDuplicateRegister( const Url& fromUrl )
{
    OsReadLock readlock( mRegisterListMutexR );
    OsWriteLock writeLock( mRegisterListMutexW );

    SipMessage* oldMsg = mRegisterList.isSameFrom( fromUrl );
    if ( oldMsg != NULL )
        return true;
    return false;
}

OsStatus 
SipRefreshMgr::sendRequest (
    SipMessage& request,
    const char* method )
{    
    OsStatus    retval = OS_UNSPECIFIED ;   // Sucess of operation
    UtlString    methodName(method) ;        // Method name fo request
    int         refreshPeriod = -1 ;        // Refresh period used when resubscribing
    UtlBoolean   bIsUnregOrUnsub ;           // Is this an unregister or unsubscribe?
   
    // Reset the transport data and the via fields
    request.resetTransport();
    request.removeLastVia();
    request.setDateField();

    bIsUnregOrUnsub = isExpiresZero(&request) ;
  

#ifdef TEST_PRINT
    {
        UtlString method2 ;
        int      cseq ;
        UtlString callId ;

        request.getCSeqField(&cseq, &method2) ;
        request.getCallIdField(&callId) ;

        if ( method2.compareTo(SIP_REGISTER_METHOD) == 0 )
        {
            osPrintf("** sendRequest cseq=%d, method=%s, callId=%s\n",
                    cseq, method2.data(), callId.data()) ;

            mRegisterList.printDebugTable() ;
        }
    }
#endif

    // Keep a copy for reschedule
    if ( !mMyUserAgent->send( request, getMessageQueue() ) )
    {
        UtlString toField ;    
        request.getToField(&toField) ;

        syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to send %s message (send failed):\nto: %s",
                method, toField.data()) ;

        // send event to upper layer.
        // check if it was unregister, if so remove from list
        // we were unregistering
        if (bIsUnregOrUnsub)
        {
            //reschedule only if expires value is not zero otherwise it means we just did an unregister
            if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
                UtlString fromUri;
                request.getFromUri(&fromUri);
                SipMessage *msgInList = mRegisterList.isSameFrom( Url(fromUri) );
                if ( msgInList )
                {
                    UtlBoolean bRemovedOk = removeFromRegisterList(msgInList);
                    if (bRemovedOk)
                    {
                        delete msgInList;
                        msgInList = NULL;
                    }
                }
            } 
            else
            {
                removeFromSubscribeList(&request);
            }
        } else // REGISTER
        {
            // @JC Added Comments: create a message on the queue with a quarter
            // lease period timeout if the timer triggers and we've not received
            // a good response from the server within FAILED_PERCENTAGE_TIMEOUT
            // secs resubscribe
            SipMessage* message = new SipMessage( request );
            if ( request.getResponseListenerData() )
            {
                message->setResponseListenerData( request.getResponseListenerData() );
            }

            // Report error to observers
            SipMessageEvent eventMsg( message );
            eventMsg.setMessageStatus( SipMessageEvent::TRANSPORT_ERROR );
            message = NULL;
            sendToObservers( eventMsg, &request );           
        }
    }
    else
    {
        retval = OS_SUCCESS;
    }


    if (!bIsUnregOrUnsub)
    {
        // Figure out the refresh period for reschedule
        if ( request.getExpiresField(&refreshPeriod) == FALSE )
        {                        
            if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
                refreshPeriod = mDefaultRegistryPeriod;
            }
            else //SUBSCRIBE
            {
                refreshPeriod = mDefaultSubscribePeriod;
            }
        }

        // Reschedule in case of failure
        rescheduleRequest(
                &request,
                refreshPeriod,
                method,
                FAILED_PERCENTAGE_TIMEOUT );
    }

    return retval;
}

void 
SipRefreshMgr::rescheduleRequest(
    SipMessage* request,
    int secondsFromNow,
    const char *method,
    int percentage,
    UtlBoolean sendImmediate)
{
    UtlString seqMethod;
    SipMessage* listMessage = NULL;
    UtlString methodStr(method);
    int defaultTime = -1; //set according to the requested method to default
    UtlString lineId;

    // Log reschedule attempt
    syslog(FAC_REFRESH_MGR, PRI_DEBUG, "rescheduling %s request:\nsecs=%d\npercent=%d\nsendNow=%d",
            method, secondsFromNow, percentage, sendImmediate) ;

    if ( methodStr.compareTo(SIP_REGISTER_METHOD) == 0 )
    {
        OsReadLock readlock(mRegisterListMutexR);
        listMessage = mRegisterList.getDuplicate(request);
        // May not have a To tag set in the list because it was sent the first time
        if ( !listMessage )
        {
            UtlString fromUri;
            request->getFromUri(&fromUri);
            Url uri(fromUri);
            uri.removeAngleBrackets();
            if ( !fromUri.isNull() )
            {
                SipMessage sipMsg;
                if ( isDuplicateRegister(uri, sipMsg) )
                {
                    listMessage = mRegisterList.getDuplicate(&sipMsg);
                }
            }
        }
        defaultTime = mDefaultRegistryPeriod;
    } else // Subscribe
    {
        OsReadLock readlock(mSubscribeListMutexR);
        listMessage = mSubscribeList.getDuplicate(request);
        // May not have a To tag set in the list because it was sent the first time
        if ( !listMessage )
        {
            UtlString fromUri;
            request->getFromUri(&fromUri);
            Url uri(fromUri);
            uri.removeAngleBrackets();
            if ( !fromUri.isNull() )
            {
                SipMessage sipMsg;
                if ( isDuplicateSubscribe(uri, sipMsg) )
                {
                    listMessage = mSubscribeList.getDuplicate(&sipMsg);
                }
            }
        }
        defaultTime = mDefaultSubscribePeriod;
    }


    // if it is an immediate send then it is either a re-register or unregister
    // The Request has already incremented CSEQ number is that case so don't
    // increment, but increase the number only to the message that is added
    // to the list because that will be used for next timer register in case
    // of re-register in case of unregister, the message will be deleted from 
    // the list upon getting a response
    if ( !sendImmediate )
    {
        request->incrementCSeqNumber();

        // Clear the DNS field, so that we retry DNS-SRV before resending.  
        // This should be performed for all failure cases, except for
        // auth challenges
        request->clearDNSField() ;
        request->resetTransport() ;
    }

    // Remove the old list message and add the new one
    if ( methodStr.compareTo(SIP_REGISTER_METHOD) == 0 )
        addToRegisterList(request);        
    else
        addToSubscribeList(request);
    
    

    // There will always be a copy - if there is no copy then don't reschedule 
    // because reregister may have removed the copy deliberately.
    if ( secondsFromNow > 0 )
    {
        request->setSendProtocol(OsSocket::UNKNOWN);
        request->setTimesSent(0);
        // add the request back to the list
        UtlString contact;
        request->getContactEntry(0,&contact);
        if ( contact.isNull() )
        {
            UtlString toField;
            UtlString contactStr;
            request->getToField(&toField);
            Url toFieldTmp(toField);
            getContactField(toFieldTmp, contactStr, lineId);
            request->setContactField(contactStr.data());
        }

        // empty the via headers
        while ( request->removeHeader(SIP_VIA_FIELD, 0) )
        {}

        // Make a copy for the timer
        SipMessage* timerRegisterMessage = new SipMessage(*request);
        OsQueuedEvent* queuedEvent = new OsQueuedEvent(mIncomingQ, (int)timerRegisterMessage);
        OsTimer* timer = new OsTimer(*queuedEvent);

        int maxSipTransactionTimeSecs = 
            (mMyUserAgent->getSipStateTransactionTimeout()/1000);

        secondsFromNow = (secondsFromNow * percentage)/100;

        // ensure that the time that the transaction times out
        // is at least the max transaction time (preventing duplicate
        // retransmits
        if ( secondsFromNow < maxSipTransactionTimeSecs )
            secondsFromNow = maxSipTransactionTimeSecs;

        // check for minumum and maximum values.
        if ( !sendImmediate )
        {
            //mseconds to seconds
            if ( secondsFromNow < maxSipTransactionTimeSecs )
            {
                secondsFromNow = maxSipTransactionTimeSecs;
            } else if ( secondsFromNow > defaultTime )
            {
                secondsFromNow = (defaultTime * percentage)/100;
            }
        }

        // Log reschedule attempt
        syslog(FAC_REFRESH_MGR, PRI_DEBUG, "rescheduled %s in %d second(s)",
                method, secondsFromNow) ;

        OsTime timerTime(secondsFromNow, 0);        
        timer->oneshotAfter(timerTime);
    }

    if (listMessage)
    {
        UtlBoolean bRemovedOk = FALSE;

        if ( methodStr.compareTo(SIP_REGISTER_METHOD) == 0 )
            bRemovedOk = removeFromRegisterList(listMessage);
        else
            bRemovedOk = removeFromSubscribeList(listMessage);

        if (bRemovedOk)
        {
            delete listMessage;
            listMessage = NULL;
        }
    }
}


void 
SipRefreshMgr::processResponse(
    const OsMsg& eventMessage,
    SipMessage *request)
{
    SipMessage* response = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
    // int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

    UtlBoolean sendEventToUpperlayer = TRUE;

#ifdef TEST_PRINT
    osPrintf("SipUserAgent::processResponse\n");
#endif

    UtlString method;
    request->getRequestMethod( &method) ;

    // ensure that this is a response first
    if ( response->isResponse() )
    {
        int responseCode = response->getResponseStatusCode();
//         osPrintf("Received response code: %d\n", responseCode);

        if ( request && responseCode < SIP_2XX_CLASS_CODE )
        {   // provisional response codes - Ignore these
#ifdef TEST_PRINT
            osPrintf("SipUserAgent::processResponse ignoring provisional response code: %d\n", responseCode);
#endif
        } else if ( (request != NULL) && 
                    ( (responseCode >= SIP_2XX_CLASS_CODE) && 
                      (responseCode < SIP_3XX_CLASS_CODE) ) )
        {   // Success Class response 2XX
            processOKResponse(response, request );
        } else if ( (request == NULL) && ( responseCode >= SIP_OK_CODE ) )
        {            
#ifdef TEST_PRINT
//            UtlString msgBody;
//            int len;
//            Response->getBytes(&msgBody, &len);
//            osPrintf("Could not find original request for response:\n%s%%%%END%%%%\n", msgBody.data());
#endif
            sendEventToUpperlayer = FALSE;
        } else if (request != NULL) // Error, try again after default times later
        {
            // unregister/unsubscribe?
            if ( isExpiresZero(request) )
            {
                UtlString method;
                request->getRequestMethod( &method) ;
                // reschedule only if expires value id not zero otherwise 
                // it means we just did an unregister
                if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
                    removeFromRegisterList(request);
                else
                    removeFromSubscribeList(request);
                sendEventToUpperlayer = FALSE;
            } else
            {
                rescheduleAfterTime(request, FAILED_PERCENTAGE_TIMEOUT);
            }
        }
        else
        {
            sendEventToUpperlayer = FALSE;
        }

        if ( sendEventToUpperlayer )
            sendToObservers(eventMessage, request);
    }
}

void 
SipRefreshMgr::processOKResponse(
    SipMessage* response, 
    SipMessage* request )
{
    int responseRefreshPeriod = -1;
    if ( !response->getExpiresField(&responseRefreshPeriod) )
    {   
        // this method expires looks at the request/response pair
        // the response may have multiple contacts so it searched
        // for the expires header corresponding to the request
        parseContactFields( response, request, responseRefreshPeriod );
    }
    int requestRefreshPeriod = -1;
    if ( !request->getExpiresField(&requestRefreshPeriod) )
    {
        // to get expires value @JC whi request 2 times
        parseContactFields( request, request, requestRefreshPeriod );
    }

#ifdef TEST_PRINT
    osPrintf("SipUserAgent::processResponce 200 ok - expiresRequest %d  expiresResponse %d\n", 
             requestRefreshPeriod, responseRefreshPeriod );
#endif

    //get to To Tag from the 200 ok response and add it to the request
    UtlString toAddr;
    UtlString toProto;
    int toPort;
    UtlString toTag;
    response->getToAddress(&toAddr, &toPort, &toProto, NULL, NULL, &toTag);

    UtlString method;
    request->getRequestMethod( &method) ;

    if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
    {
        //reschedule only if expires value id not zero otherwise it means we just did an unregister
        if ( requestRefreshPeriod == 0 )
        {
            removeFromRegisterList(request);
        } else if ( responseRefreshPeriod > 0 )
        {
            if ( !toTag.isNull() )
            {
                request->setToFieldTag(toTag);
            }
            rescheduleRequest(request, responseRefreshPeriod, SIP_REGISTER_METHOD);
        } else // could not find expires in 200 ok response , reschedule after default time
        {   // copying from response (this is why we set the To Field
            request->setToFieldTag(toTag);
            rescheduleAfterTime(request);
        }
    } else // subscribe
    {
        // reschedule according to expires value
        if ( requestRefreshPeriod == 0 )
        {
            removeFromSubscribeList(request);
        } else if ( responseRefreshPeriod > 0 )
        {
            if ( !toTag.isNull() )
            {
                request->setToFieldTag(toTag);
            }

            rescheduleRequest(
                request, 
                responseRefreshPeriod, 
                SIP_SUBSCRIBE_METHOD);
        } else // could not find expires in 200 ok response , reschedule after default time
        {   // copying from response (this is why we set the To Field
            request->setToFieldTag(toTag);
            rescheduleAfterTime(request);
        }
    }
}

void 
SipRefreshMgr::parseContactFields(
    SipMessage* registerResponse, 
    SipMessage* requestMessage, 
    int &serverRegPeriod)
{
    // get the request contact value ...so that we can find out 
    // the expires subfield value for this contact from the list 
    // of contacts returned by the registration server
    UtlString requestContactValue;
    requestMessage->getContactEntry(0 , &requestContactValue);

    UtlString contactField;
    int indexContactField = 0;

    while ( registerResponse->getContactEntry(indexContactField , &contactField) )
    {
        if ( strstr(contactField, requestContactValue ) != NULL )
        {
            UtlString subfieldText;
            int subfieldIndex = 0;
            UtlString subfieldName;
            UtlString subfieldValue;
            NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
            while ( !subfieldText.isNull() )
            {
                NameValueTokenizer::getSubField(subfieldText.data(), 0, "=", &subfieldName);
                NameValueTokenizer::getSubField(subfieldText.data(), 1, "=", &subfieldValue);
#ifdef TEST_PRINT
                osPrintf("SipUserAgent::processRegisterResponce found contact parameter[%d]: \"%s\" value: \"%s\"\n",
                         subfieldIndex, subfieldName.data(), subfieldValue.data());
#endif
                subfieldName.toUpper();
                if ( subfieldName.compareTo(SIP_EXPIRES_FIELD) == 0 )
                {

                    //see if more than one token in the expire value
                    NameValueTokenizer::getSubField(
                        subfieldValue, 1,
                        " \t:;,", &subfieldText);

                    // if not ...time is in seconds
                    if ( subfieldText.isNull() )
                    {
                        serverRegPeriod = atoi(subfieldValue);
                    }
                    // If there is more than one token assume it is a text date
                    else
                    {
                        // Get the expiration date
                        long dateExpires = OsDateTime::convertHttpDateToEpoch(subfieldValue);
                        long dateSent = 0;
                        // If the date was not set in the message
                        if ( !registerResponse->getDateField(&dateSent) )
                        {
#ifdef TEST_PRINT
                            osPrintf("Date field not set\n");
#endif
                            // Assume date sent is now
                            dateSent = OsDateTime::getSecsSinceEpoch();
                        }
#ifdef TEST_PRINT
                        osPrintf("Contact expires date: %ld\n", dateExpires); osPrintf("Current time: %ld\n", dateSent);
#endif
                        serverRegPeriod = dateExpires - dateSent;
                    }
                    break;
                }
                subfieldIndex++;
                NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
            }
        }
        indexContactField ++;
    }
    return ;

}

void 
SipRefreshMgr::sendToObservers (
    const OsMsg& eventMessage,
    SipMessage *request )
{

    SipMessage* Response = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
    int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

    // Create a new message event
    SipMessage * message = new SipMessage(*Response);

    SipMessageEvent event(message);
    event.setMessageStatus(messageType);

    UtlString method;
    request->getRequestMethod( &method) ;
    // Find all of the observers which are interested in
    // this method and post the message
    if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
    {
        queueMessageToObservers(event, SIP_REGISTER_METHOD);
    } else
    {
        queueMessageToObservers(event, SIP_SUBSCRIBE_METHOD);
    }
    // send it to those with no method descrimination as well
    queueMessageToObservers(event, "");
    // Do not delete the message it gets deleted with the event
}

void 
SipRefreshMgr::registerUrl(
    const char* registerFromAddress,
    const char* registerToAddress,
    const char* registerUri,
    const UtlString& registerCallId,
    const UtlString& lineId,
    int registerPeriod)
{
    SipMessage* regMessage = new SipMessage();
    UtlString contactField;
    Url registerToAddressTmp(registerToAddress);
    getContactField( registerToAddressTmp, contactField, lineId );

    UtlString fromField(registerFromAddress);
    // add Tag to from field
    UtlString tagNamevaluePair ;
    createTagNameValuePair(tagNamevaluePair);
    fromField.append(";");
    fromField.append(tagNamevaluePair);

    regMessage->setRegisterData (
        fromField.data(),       // from
        registerToAddress,      // to
        registerUri,            // uri
        contactField.data(),    // contact
        registerCallId.data(),  // callid
        1,
        registerPeriod >= 0 ? registerPeriod : mDefaultRegistryPeriod );


    // Add pto the register list
    addToRegisterList(regMessage);
    

    // If the user agent is not started, then queue it.  Once the
    // UA is started, it will kick off those registrations.
    if ( isUAStarted() )
    {       
        if (sendRequest(*regMessage , SIP_REGISTER_METHOD) != OS_SUCCESS)
            removeFromRegisterList(regMessage);

    }
    else
    {        
        regMessage->setContactField("") ; // BA: Why we are clearing the contact field?

        syslog(FAC_REFRESH_MGR, PRI_DEBUG, "queueing register until the SIP UA is ready:\nfrom=%s\nto=%s\nuri=%s\ncontact=%s\ncallid=%s\nlineid=%s",
                fromField.data(), registerToAddress, registerUri, 
                contactField.data(), registerCallId.data(), lineId.data()) ;
    }
}


UtlBoolean 
SipRefreshMgr::handleMessage( OsMsg& eventMessage )
{
    UtlBoolean messageProcessed = FALSE;
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    UtlString method;

    if ( msgType == OsMsg::REFRESH_MSG && msgSubType == SipRefreshMgr::START_REFRESH_MGR )
    {
        waitForUA();
        messageProcessed = TRUE;
    }

    if ( msgType == OsMsg::PHONE_APP )
    {
        SipMessage* sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
        int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
        UtlString callid;
        int cseq;
        sipMsg->getCallIdField(&callid);
        sipMsg->getCSeqField(&cseq, &method);        

#ifdef TEST_PRINT
        if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
        {
            int len ;
            UtlString bytes ;

            osPrintf("** handleMessage cseq=%d, method=%s, callid=%s\n", cseq, method.data(), callid.data()) ;
            sipMsg->getBytes(&bytes, &len) ;
            osPrintf("%s\n", bytes.data()) ;            
            mRegisterList.printDebugTable() ;             
        }
#endif

        // if transport error and no response from remote machine. Unable to send to remote host
        if ( !sipMsg->isResponse() && messageType == SipMessageEvent::TRANSPORT_ERROR )
        {            
            SipMessage * msgInList = NULL;
            UtlBoolean bRemovedOk = FALSE;
            
            // Log Failures
            syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to send %s (transport):\ncallid=%s",
                    method.data(), callid.data()) ;

            if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
                //remove using call id instead of from field because the from in
                //startup unregister is same as the actual register request.

               SipMessage *pTmpMessage = mRegisterList.isSameCallId(callid);

                // make a copy in case the message in list is removed
                if ( pTmpMessage )
                    msgInList = new SipMessage(*pTmpMessage);
            }
            else
            {
                //remove using call id instead of from field because the from in
                //startup unregister is same as the actual register request.
                SipMessage *pTmpMessage = mSubscribeList.isSameCallId(callid);

                // make a copy in case the message in list is removed
                if ( pTmpMessage )
                    msgInList = new SipMessage(*pTmpMessage);
            }

            
            //reschedule only if expires value is not zero otherwise it means we just did an unregister
            if ( isExpiresZero(sipMsg) ) 
            {                
                if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
                { 
                    bRemovedOk = removeFromRegisterList(msgInList);
                } 
                else
                {
                    bRemovedOk = removeFromSubscribeList(msgInList);
                }
            } 
            else
            {
               sendToObservers(eventMessage, msgInList);
                    
               // try again after default time out
               rescheduleAfterTime(msgInList, FAILED_PERCENTAGE_TIMEOUT);
            }

            if (msgInList && bRemovedOk)
            {
                delete msgInList;
                msgInList = NULL;
            }

            messageProcessed = TRUE;
        }
        // If this is a response,
        else if ( sipMsg->isResponse() )
        {
            SipMessage *request = NULL;
            SipMessage *requestFound = NULL;

            // Is this a register msg?
            {
                OsReadLock readlock(mRegisterListMutexR);
                // Find the request which goes with this response
                requestFound = mRegisterList.getRequestFor(sipMsg);
                //make a dupe
                if ( requestFound )
                    request = new SipMessage(*requestFound);
            }

            // IS this a subscribe msg?
            if ( !request )
            {
                OsReadLock readlock(mSubscribeListMutexR);
                // Find the request which goes with this response
                requestFound = mSubscribeList.getRequestFor(sipMsg);
                if (requestFound)
                    request = new SipMessage(*requestFound);
            }

            if ( request )
            {
                UtlBoolean retryWithAuthentication = FALSE;
                request->getRequestMethod( &method) ;

                if ( messageType == SipMessageEvent::AUTHENTICATION_RETRY )
                {
                    syslog(FAC_REFRESH_MGR, PRI_INFO, "authentication requested for %s request:\ncallid=%s",
                            method.data(), callid.data()) ;

                    if ( strcmp(method.data(), SIP_REGISTER_METHOD) == 0 )
                    {
                        // Find the request which goes with this response
                        SipMessage* request = mRegisterList.getRequestFor(sipMsg);

                        UtlBoolean bRemovedOk = removeFromRegisterList(request);
                        
                        // increment the CSeq number in the stored request
                        request->incrementCSeqNumber();
                        addToRegisterList(request);
                        
                        // Clean up
                        if (bRemovedOk)
                        {
                            delete request;
                            request = NULL;
                        }
                        retryWithAuthentication = TRUE;
                    } 
                    else if ( strcmp(method.data(), SIP_SUBSCRIBE_METHOD) == 0 )
                    {
                        // Find the request which goes with this response
                        SipMessage* request = mSubscribeList.getRequestFor(sipMsg);
                        UtlBoolean bRemovedOk = removeFromSubscribeList(request);

                        //increment the CSeq number in the stored request
                        request->incrementCSeqNumber();
                        addToSubscribeList(request);

                        // Clean up
                        if (bRemovedOk)
                        {
                            delete request;
                            request = NULL;
                        }
                        retryWithAuthentication = TRUE;
                    }
                }

                if ( request && retryWithAuthentication == FALSE )
                {
                    processResponse(eventMessage, request);
                }                
            } 
            else
            {

                // Bob 2/10/03 Do not complain if we cannot find the 
                // message.  Because we add observers for both requests
                // and incoming messages, we *WILL* receive duplicate
                // responses- and yes, we won't find them here.
/*
                // Report that we were unable to find this request
                UtlString response ;
                int      respLen ;
                UtlString msgContents ;

                // Log Failure
                sipMsg->getBytes(&response, &respLen) ;                
                dumpMessageLists(msgContents) ;                
                syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to find request for %s response:\ncallid=%s\nResponse:\n%s\nLists:\n%s",
                    method.data(), callid.data(), response.data(), msgContents.data()) ;
*/

            }

            if ( request )
            {
                delete request;
                request = NULL;
            }
        }//end if isresponse


        messageProcessed = TRUE;
    } 
    else if ( (msgType == OsMsg::OS_EVENT) && (msgSubType == OsEventMsg::NOTIFY) )
    {   
        // A timer expired
        SipMessage* sipMessage;
        OsTimer* timer;
        OsQueuedEvent* notifier;
        int protocolType;

        ((OsEventMsg&)eventMessage).getUserData((int&)sipMessage);
        ((OsEventMsg&)eventMessage).getEventData((int&)timer);
        notifier = (OsQueuedEvent*) timer->getNotifier();

        if ( timer )
        {
            delete timer;
            timer = NULL;
        }

        if ( notifier )
        {
            delete notifier;
            notifier = NULL;
        }

        if ( sipMessage )
        {
            UtlString callId;
            protocolType = sipMessage->getSendProtocol();
            sipMessage->getCallIdField(&callId);
            sipMessage->getRequestMethod(&method);

            // Log Timeout
            syslog(FAC_REFRESH_MGR, PRI_DEBUG, "timeout for %s:\ncallid=%s",
                    method.data(), callId.data())  ;

#ifdef TEST_PRINT
            int len = 0 ;
            UtlString bytes ;

            sipMessage->getBytes(&bytes, &len) ;
            osPrintf("%s\n", bytes.data()) ;
#endif
            
            // check if a duplicate request is in the list, 
            // if not then it means that it was unregistered 
            // before the timer expired
            UtlString fromUri;
            sipMessage->getFromUri(&fromUri);
            Url uri(fromUri);
            uri.removeAngleBrackets();

            SipMessage sipMsg;
            if ( !fromUri.isNull() )
            {
                int num;
                UtlString method;
                sipMessage->getCSeqField(&num , &method);

                if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
                {
                    if ( isDuplicateRegister(uri, sipMsg) )
                    {
                        int listNum;
                        UtlString listMethod;
                        sipMsg.getCSeqField(&listNum , &listMethod);

                        // check if CSeq is less than what is in the list ..if less, then it is because
                        // reregister must have incremented it and this rescheduling from the previous
                        // msg
                        if ( num >= listNum )
                        {                            
                            if (sendRequest(*sipMessage, SIP_REGISTER_METHOD) != OS_SUCCESS)
                                removeFromRegisterList(sipMessage);

                        }
                    }
                    else
                    {
                        syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to refresh %s (not found):\ncallid=%s",
                            method.data(), callId.data()) ;                  
                    }
                } 
                else
                {
                    if ( isDuplicateSubscribe(uri, sipMsg) )
                    {
                        int listNum;
                        UtlString listMethod;
                        sipMsg.getCSeqField(&listNum , &listMethod);

                        // check if CSeq is less than what is in the list ..if less, then it is because
                        // reregister must have incremented it and this rescheduling from the previous
                        // msg
                        if ( num >= listNum )
                        {                            
                            if (sendRequest(*sipMessage, SIP_SUBSCRIBE_METHOD) != OS_SUCCESS)
                                removeFromSubscribeList(sipMessage);

                        }
                    }
                    else
                    {
                        syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to refresh %s (not found):\ncallid=%s",
                            method.data(), callId.data()) ;                  
                    }                     
                }
            }

            // The timer made its own copy of this message.Delete it now that we are done with it.
            delete sipMessage;
            sipMessage = NULL;            

            messageProcessed = TRUE;
        }
    }

    return (messageProcessed);
}

void 
SipRefreshMgr::queueMessageToObservers(
    SipMessageEvent& event, 
    const char* method )
{
    const SipMessage* message = event.getMessage();

    // Find all of the observers which are interested in this method and post the message
    UtlString messageEventName;
    if ( message ) message->getEventField(messageEventName);
    UtlString observerMatchingMethod(method);
    SipObserverCriteria* observerCriteria = NULL;

    OsReadLock lock(mObserverMutex);

    UtlHashBagIterator observerIterator(mMessageObservers, &observerMatchingMethod);
    do
    {
        observerCriteria = (SipObserverCriteria*) observerIterator();

        // If this message matches the filter criteria
        //wants responses
        if ( observerCriteria &&
             observerCriteria->wantsIncoming() &&
             (message->isResponse() && observerCriteria->wantsResponses()) )
        {
            OsMsgQ* observerQueue = observerCriteria->getObserverQueue();
            void* observerData = observerCriteria->getObserverData();

            // Cheat a little and set the observer data to be passed back
            ((SipMessage*) message)->setResponseListenerData(observerData);

            // Put the message in the observers queue
            observerQueue->send(event);
        }
        //wants requests
        if ( observerCriteria &&
             observerCriteria->wantsIncoming() &&
             !message->isResponse() )
        {
            OsMsgQ* observerQueue = observerCriteria->getObserverQueue();
            void* observerData = observerCriteria->getObserverData();

            // Cheat a little and set the observer data to be passed back
            ((SipMessage*) message)->setResponseListenerData(observerData);

            // Put the message in the observers queue
            observerQueue->send(event);
        }

    }
    while ( observerCriteria != NULL );
}

/*===================================================================*/
// SUBSCRIBE
/*===================================================================*/
void SipRefreshMgr::reSubscribeAll()
{
    SipMessageList tempList;
    SipMessage* listMessage;
    int iteratorHandle;

    {//scope the locks
        OsReadLock readlock(mSubscribeListMutexR);
        OsWriteLock writeLock(mSubscribeListMutexW);

        iteratorHandle = mSubscribeList.getIterator();
        while ((listMessage = (SipMessage*) mSubscribeList.getSipMessageForIndex(iteratorHandle)))
        {
            //somehow stop the old timer - will get not cause harm because will not be able to find corresponding request.
            tempList.add(listMessage);
        }
        mSubscribeList.releaseIterator(iteratorHandle);
    }

    iteratorHandle = tempList.getIterator();
    while ((listMessage = (SipMessage*) tempList.getSipMessageForIndex(iteratorHandle)))
    {
        //removeFromSubscribeList(listMessage);
        rescheduleRequest(listMessage, 1 , SIP_SUBSCRIBE_METHOD, DEFAULT_PERCENTAGE_TIMEOUT ,TRUE);
    }
    tempList.releaseIterator(iteratorHandle);
}

void SipRefreshMgr::unSubscribeAll()
{
    SipMessage* listMessage;
    int iteratorHandle;

    {//scope the locks
        OsReadLock readlock(mSubscribeListMutexR);
        OsWriteLock writeLock(mSubscribeListMutexW);

        iteratorHandle = mSubscribeList.getIterator();
        while ((listMessage = (SipMessage*) mSubscribeList.getSipMessageForIndex(iteratorHandle)))
        {
            listMessage->setExpiresField(0);
            listMessage->incrementCSeqNumber();

            // Clear the DNS field, so that we retry DNS-SRV before resending.  
            // This should be performed for all failure cases, except for
            // auth challenges
            listMessage->clearDNSField() ;
            listMessage->resetTransport() ;
            mMyUserAgent->send(*listMessage);
            mSubscribeList.remove(iteratorHandle);

            delete listMessage;
        }
        mSubscribeList.releaseIterator(iteratorHandle);
    }

}

UtlBoolean 
SipRefreshMgr::newSubscribeMsg( SipMessage& sipMessage )
{
    UtlString to;
    UtlString method;
    UtlString eventType;

    // Assume failed unless proven otherwise
    UtlBoolean result = false;    
    sipMessage.getRequestMethod( &method );        
    sipMessage.getToField( &to );
    sipMessage.getEventField( eventType );

    // ensure that this is a subscribe request
    if ( strcmp( method.data(), SIP_SUBSCRIBE_METHOD ) == 0 )
    {
        if ( !isDuplicateSubscribe( to.data() ) )
        {
            syslog(FAC_REFRESH_MGR, PRI_DEBUG, "adding registration:\nurl=%s\nevent-type=%s",
                    to.data(), eventType.data()) ;

            // lazy create the from tag
            Url fromUrl;
            sipMessage.getFromUrl( fromUrl );
            UtlString tagValue;
            if ( !fromUrl.getFieldParameter( "tag", tagValue ) )
            {
                // tag has not been set so set it here
                UtlString tagNameValuePair;
                createTagNameValuePair( tagNameValuePair );
                // Tokenize out the tag=value part
                UtlTokenizer next (tagNameValuePair);
                UtlString junk;
                next.next(junk, "=");
                UtlString tagValue;
                next.next(tagValue, "=");
                fromUrl.setFieldParameter("tag", tagValue);
                sipMessage.setRawFromField( fromUrl.toString().data() );
            }

            // lazy create the callid
            UtlString subscribeCallId;
            sipMessage.getCallIdField(&subscribeCallId);
            if ( subscribeCallId.isNull() )
            {
                UtlString from;
                sipMessage.getFromField( &from );
                // Generate Call Id
                UtlString subscribeCallId;
                generateCallId ( 
                    from,
                    SIP_SUBSCRIBE_METHOD, 
                    subscribeCallId );

                sipMessage.setCallIdField(
                    subscribeCallId.data());
            }

            // Add to subscription list
            addToSubscribeList( &sipMessage );

            if (isUAStarted())
            {
                if (sendRequest( sipMessage, SIP_SUBSCRIBE_METHOD ) != OS_SUCCESS)
                    removeFromSubscribeList(&sipMessage);

            }
            else
            {
                syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to add registration (queue):\nurl=%s\nevent-type=%s",
                        to.data(), eventType.data()) ;

            } 
            result = true;
        } 
        else
        {
            syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to add new registration (dup):\nurl=%s\nevent-type=%s",
                    to.data(), eventType.data()) ;
        }
    } 
    else
    {
        osPrintf("ERROR SipRefreshMgr::newSubscribeMsg - "
                 "Not a SIP_SUBSCRIBE_METHOD request\n");
    }
    return result;
}


// Appends the message contents of both the mRegisterList and mSubscribeList.
void SipRefreshMgr::dumpMessageLists(UtlString& results)
{
    UtlString temp ;

    // Dump Register List
    results.append("\nRegister List:\n\n") ;
    mRegisterList.remove(0) ;
    mRegisterList.toString(temp) ;
    results.append(temp) ;

    // Dump Subscribe List
    results.append("\nSubscribe List:\n\n") ;
    mRegisterList.remove(0) ;
    mSubscribeList.toString(temp) ;
    results.append(temp) ;
}


void 
SipRefreshMgr::generateCallId(
    const UtlString& fromUrl,
    const UtlString& method,
    UtlString& callId,
    UtlBoolean onStartup)
{
    Url temp(fromUrl);
    UtlString identity;
    temp.getIdentity(identity);

    if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
    {
        if ( onStartup )
        {
            int previousCount = mRestartCount -1;
            char strCount[64];
            sprintf(strCount, "%d", previousCount);

            //use previous restart count to get the previous call id
            HttpMessage::buildMd5UserPasswordDigest(mMacAddress, strCount, identity, callId);
        } else
        {
            HttpMessage::buildMd5UserPasswordDigest(mMacAddress, mRestartCountStr, identity, callId);
        }
    } else // subscribe method
    {
        // call id is a hash of fromUrl, ipaddress+rnd, method
        int randomInt = rand();
        char randomString[60];
        sprintf (randomString, "-%d", randomInt);

        UtlString randomizedHostIP;
        OsSocket::getHostIp( &randomizedHostIP );
        randomizedHostIP.append (randomString);

        HttpMessage::buildMd5UserPasswordDigest(
            fromUrl, randomizedHostIP, method, callId );
    }
}

UtlBoolean 
SipRefreshMgr::removeFromRegisterList(SipMessage* message)
{
    UtlBoolean bRemovedOk = FALSE;

    OsReadLock readlock(mRegisterListMutexR);
    OsWriteLock writeLock(mRegisterListMutexW);
#ifdef TEST_PRINT
    osPrintf("**********************************************\n");
    osPrintf("Removing message from register list: %X\n",message);
    osPrintf("**********************************************\n");
#endif
    if ( mRegisterList.remove(message) == FALSE )
    {
#ifdef TEST_PRINT
        osPrintf("**********************************************\n");
        osPrintf("Cannot Find message to register list: %X\n",message);
        osPrintf("**********************************************\n");
#endif
    }
    else
        bRemovedOk = TRUE;

#ifdef TEST_PRINT
    osPrintf("** removeFromRegisterList\n") ;
    mRegisterList.printDebugTable() ;
#endif

    return bRemovedOk;
}

void 
SipRefreshMgr::addToRegisterList(SipMessage *message)
{
    OsReadLock readlock(mRegisterListMutexR);
    OsWriteLock writeLock(mRegisterListMutexW);

    SipMessage *msg = new SipMessage (*message);
#ifdef TEST_PRINT
    osPrintf("**********************************************\n");
    osPrintf("Adding message to register list: orig: %X new: %X\n",message, msg);
    osPrintf("**********************************************\n");
#endif
    mRegisterList.add(msg);

#ifdef TEST_PRINT
    osPrintf("** addToRegisterList\n") ;
    mRegisterList.printDebugTable() ;
#endif
}

UtlBoolean  
SipRefreshMgr::removeFromSubscribeList(SipMessage* message)
{
    UtlBoolean bRemovedOk = FALSE;

    OsReadLock readlock(mSubscribeListMutexR);
    OsWriteLock writeLock(mSubscribeListMutexW);
#ifdef TEST_PRINT
    osPrintf("**********************************************\n");
    osPrintf("Removing message from subscribe list: %X\n",message);
    osPrintf("**********************************************\n");
#endif
    if (mSubscribeList.remove(message) == FALSE)
    {
#ifdef TEST_PRINT
        osPrintf("**********************************************\n");
        osPrintf("Cannot Find message to subscribe list: %X\n",message);
        osPrintf("**********************************************\n");
#endif
    }
    else
        bRemovedOk = TRUE;

    return bRemovedOk;
}

void 
SipRefreshMgr::addToSubscribeList(SipMessage *message)
{
    OsReadLock readlock(mSubscribeListMutexR);
    OsWriteLock writeLock(mSubscribeListMutexW);
    mSubscribeList.add(new SipMessage (*message));
}

UtlBoolean 
SipRefreshMgr::isDuplicateSubscribe ( const Url& toUrl )
{
    OsReadLock readlock(mSubscribeListMutexR);
    OsWriteLock writeLock(mSubscribeListMutexW);

    SipMessage * oldMsg = mSubscribeList.isSameTo(toUrl);
    if ( oldMsg )
    {
        return true;
    }
    return false;
}

UtlBoolean 
SipRefreshMgr::isDuplicateSubscribe (
    const Url& fromUrl, 
    SipMessage &oldMsg )
{
    OsReadLock readlock(mSubscribeListMutexR);
    OsWriteLock writeLock(mSubscribeListMutexW);
    // call copy constructor on the oldMsg
    SipMessage* pduplicate = mSubscribeList.isSameFrom(fromUrl);
    if ( pduplicate != NULL )
    {
        oldMsg = *pduplicate;
        return true;
    }
    return false;
}

void 
SipRefreshMgr::rescheduleAfterTime(
    SipMessage *request, 
    int percentage )
{
    int iOriginalExpiration ;
    UtlString method ;
    request->getRequestMethod(&method);
    
    // Figure out expiration
    if (!request->getExpiresField(&iOriginalExpiration))
    {
        if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
        {
            iOriginalExpiration = mDefaultRegistryPeriod ;
        }
        else
        {
            iOriginalExpiration = mDefaultSubscribePeriod ;
        } 
    }
        
    rescheduleRequest( 
        request, 
        iOriginalExpiration, 
        method.data(), 
        percentage);
}

void 
SipRefreshMgr::createTagNameValuePair( UtlString& tagNamevaluePair )
{
    tagNamevaluePair.remove(0);
    // Build a from tag
    int fromTagInt = rand();
    char fromTagBuffer[60];
    sprintf(fromTagBuffer, "%dc%d", fromTagInt, fromTagInt);
    tagNamevaluePair.append("tag=");
    tagNamevaluePair.append(fromTagBuffer);
}

void 
SipRefreshMgr::getContactField(
    const Url& registerToField, 
    UtlString& contact,
    const UtlString& lineId )
{
    contact.remove(0);
    UtlString tempContact;
    UtlString displayName;
    UtlString userId;

    mMyUserAgent->getContactUri(&tempContact);

    //the contact URi does not have the correct urserId information in it ...
    //get the user ID and display name from To field and stick it in
    Url contactUrl(tempContact);
    registerToField.getDisplayName(displayName);
    registerToField.getUserId(userId);

    contactUrl.setDisplayName(displayName);
    contactUrl.setUserId(userId);
    if ( !lineId.isNull() )
    {
        contactUrl.setUrlParameter(SIP_LINE_IDENTIFIER, lineId);
        contactUrl.includeAngleBrackets();
    }

    int index = 0;
    UtlString paramName;
    UtlString paramValue;

    while ( ((Url&)(registerToField)).getFieldParameter( index, paramName , paramValue ) )
    {
        if ( paramName.compareTo(SIP_Q_FIELD, UtlString::ignoreCase) == 0 )
        {
            contactUrl.setFieldParameter(SIP_Q_FIELD, paramValue);
        }
        index ++;
    }
    contact.append(contactUrl.toString().data());
}



// Is the expire field set to zero for the specified request?
UtlBoolean SipRefreshMgr::isExpiresZero(SipMessage* pRequest) 
{
    UtlBoolean bRC = FALSE ;     // Is the expire field zero?

    if ((pRequest != NULL) && !pRequest->isResponse())
    {
        int iExpires = -1;
        if (!pRequest->getExpiresField(&iExpires))
        {
            parseContactFields(pRequest, pRequest, iExpires) ;
        }

        if (iExpires == 0)
            bRC = TRUE ;
    }

    return bRC ;
}

