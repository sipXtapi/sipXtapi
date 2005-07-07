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
#include <assert.h>

// APPLICATION INCLUDES
#include "AuthProxyCseObserver.h"
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsQueuedEvent.h>
#include "os/OsEventMsg.h"
#include <os/OsTimer.h>
#include <os/OsSysLog.h>

//#define TEST_PRINT 1
#define LOG_DEBUG 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int AuthProxyCallStateFlushInterval = 20; /* seconds */

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AuthProxyCseObserver::AuthProxyCseObserver(SipUserAgent&    sipUserAgent,
                                           const UtlString& dnsName,
                                           OsFile*          eventFile
                                           ) :
   OsServerTask("AuthProxyCseObserver-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mBuilder(dnsName),
   mEventFile(eventFile),
   mSequenceNumber(0)
{
   OsTime timeNow;
   OsDateTime::getCurTime(timeNow);
   
   mBuilder.observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset,
                         "AuthProxyCseObserver");
   UtlString event;
   mBuilder.xmlElement(event);

   unsigned long written;
   OsStatus writeStatus = mEventFile->write(event.data(), event.length(), written);
   if (OS_SUCCESS != writeStatus)
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "AuthProxyCseObserver initial event log write failed %d",
                    writeStatus
                    );
   }
   mEventFile->flush(); // try to ensure that at least the sequence restart gets to the file

   // get my inbound OsMsg queue
   OsMsgQ* myTaskQueue = getMessageQueue();

   // set up periodic timer to flush log file
   OsQueuedEvent* pEvent = new OsQueuedEvent(*myTaskQueue, 0);
   mFlushTimer = new OsTimer(*pEvent) ;
   mFlushTimer->periodicEvery(OsTime(), OsTime(AuthProxyCallStateFlushInterval, 0)) ;

  // Register to get incoming requests
   sipUserAgent.addMessageObserver(*myTaskQueue,
                                   SIP_ACK_METHOD,
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
   sipUserAgent.addMessageObserver(*myTaskQueue,
                                   SIP_BYE_METHOD,
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
   sipUserAgent.addMessageObserver(*myTaskQueue,
                                   SIP_INVITE_METHOD,
                                   TRUE, // Requests,
                                   TRUE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, // any session
                                   NULL // no observerData
                                   );
}

// Destructor
AuthProxyCseObserver::~AuthProxyCseObserver()
{
   mEventFile->flush(); // try to get buffered records to the file...
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean AuthProxyCseObserver::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();
   switch (msgType)
   {
   case OsMsg::OS_EVENT:
      switch (eventMessage.getMsgSubType())
      {
      case OsEventMsg::NOTIFY:
         mEventFile->flush(); // try to get buffered records to the file...
         break;
      }
      break ;
      
   case OsMsg::PHONE_APP:
   {
      SipMessage* sipMsg;

      if(SipMessageEvent::TRANSPORT_ERROR == ((SipMessageEvent&)eventMessage).getMessageStatus())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "AuthProxyCseObserver::handleMessage transport error");
      }
      else if((sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage()))
      {
         UtlString method;
         int       rspStatus = 0;
         UtlString rspText;
         UtlString contact;

         enum 
            {
               UnInteresting,
               aCallSetup,
               aCallFailure,
               aCallEnd
            } thisMsgIs = UnInteresting;
         
         if (!sipMsg->isResponse())
         {
            // sipMsg is a Request
            sipMsg->getRequestMethod(&method);

            if (0==method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase))
            {
               thisMsgIs = aCallSetup;
               sipMsg->getContactEntry(0, &contact);
            }
            if (0==method.compareTo(SIP_BYE_METHOD, UtlString::ignoreCase))
            {
               thisMsgIs = aCallEnd; // no additional information needed
            }
            else
            {
               // other request methods are not interesting
            }
         }
         else 
         {
            /*****************
             * At present, none of this happens.  It turns out that in a proxy
             * the SipUserAgent will not return responses - they are forwarded
             * at a lower layer in the stack and don't make it up to the
             * queueMessageToObservers routine.
             *
             * For the time being, this means that we don't see what we need to
             * generate CallFailure events.  The code to recognize and record them
             * has been left here in case we need it and figure out where/how
             * to get the right messages dispatched to it.
             *****************/
            int seq;
            
            if (sipMsg->getCSeqField(&seq, &method))
            {
               if (0==method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
               {
                  // this is an INVITE response - might be either a Failure or a Setup

                  rspStatus = sipMsg->getResponseStatusCode();
                  if (   (rspStatus >= SIP_4XX_CLASS_CODE) // any failure
                      // except for these three
                      && ! (   (rspStatus == HTTP_UNAUTHORIZED_CODE)
                            || (rspStatus == HTTP_PROXY_UNAUTHORIZED_CODE)
                            || (rspStatus == SIP_REQUEST_TIMEOUT_CODE)
                            )
                      )
                  {
                     // a final failure - this is a CallFailure
                     thisMsgIs = aCallFailure;
                     sipMsg->getResponseStatusText(&rspText);
                  }
               }
               else
               {
                  // responses to non-INVITES are not interesting
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver - no Cseq in response");
            }
         }

#        ifdef LOG_DEBUG
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "AuthProxyCseObserver message is %s",
                       (  thisMsgIs == UnInteresting ? "UnInteresting"
                        : thisMsgIs == aCallEnd      ? "a Call End"
                        : thisMsgIs == aCallFailure  ? "a Call Failure"
                        : thisMsgIs == aCallSetup    ? "a Call Setup"
                        : "BROKEN"
                        )); 
#        endif

         if (thisMsgIs != UnInteresting)
         {
            // collect the sequence data
            mSequenceNumber++;
            
            OsTime timeNow;
            OsDateTime::getCurTime(timeNow); 

            // collect the dialog information
            UtlString callId;
            sipMsg->getCallIdField(&callId);
         
            Url toUrl;
            sipMsg->getToUrl(toUrl);
            UtlString toTag;
            toUrl.getFieldParameter("tag", toTag);

            Url fromUrl;
            sipMsg->getFromUrl(fromUrl);
            UtlString fromTag;
            fromUrl.getFieldParameter("tag", fromTag);

            // collect the To and From
            UtlString toField;
            sipMsg->getToField(&toField);
            
            UtlString fromField;
            sipMsg->getFromField(&fromField);

            // generate the call state event record
            switch (thisMsgIs)
            {
            case aCallSetup:
               mBuilder.callSetupEvent(mSequenceNumber, timeNow, contact);
               break;

            case aCallFailure:
               mBuilder.callFailureEvent(mSequenceNumber, timeNow, rspStatus, rspText);
               break;
               
            case aCallEnd:
               mBuilder.callEndEvent(mSequenceNumber, timeNow);
               break;

            default:
               // shouldn't be possible to get here
               OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver invalid thisMsgIs");
               break;
            }

            mBuilder.addCallData(callId, fromTag, toTag, fromField, toField);

            UtlString via;
            for (int i=0; sipMsg->getViaField(&via, i); i++)
            {
               mBuilder.addEventVia(via);
            }

            mBuilder.completeCallEvent();
            
            // get the completed record
            UtlString event;
            mBuilder.xmlElement(event);

            // and write it to the file
            unsigned long written;
            mEventFile->write(event.data(), event.length(), written);
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver getMessage returned NULL");
      }
   }
   break;
   
   default:
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "AuthProxyCseObserver invalid message type %d", msgType );
   }
   } // end switch (msgType)
   
   return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

