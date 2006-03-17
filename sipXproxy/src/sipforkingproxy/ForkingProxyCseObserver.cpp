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
#include "ForkingProxyCseObserver.h"
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsQueuedEvent.h>
#include "os/OsEventMsg.h"
#include <os/OsSysLog.h>

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int ForkingProxyCallStateFlushInterval = 20; /* seconds */

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ForkingProxyCseObserver::ForkingProxyCseObserver(SipUserAgent&         sipUserAgent,
                                                 const UtlString&      dnsName,
                                                 CallStateEventWriter* pWriter
                                                 ) :
   OsServerTask("ForkingProxyCseObserver-%d", NULL, 2000),
   mpSipUserAgent(&sipUserAgent),
   mpBuilder(NULL),
   mpWriter(pWriter),
   mSequenceNumber(0)
{
   OsTime timeNow;
   OsDateTime::getCurTime(timeNow);
   UtlString event;

   if (mpWriter)
   {
      switch (pWriter->getLogType())
      {
      case CallStateEventWriter::CseLogFile:
         mpBuilder = new CallStateEventBuilder_XML(dnsName);
         break;
      case CallStateEventWriter::CseLogDatabase:
         mpBuilder = new CallStateEventBuilder_DB(dnsName);
         break;
      }
      if (mpBuilder)
      {
         if (pWriter->openLog())
         {
            mpBuilder->observerEvent(mSequenceNumber, timeNow, CallStateEventBuilder::ObserverReset,
                                     "ForkingProxyCseObserver");      
            mpBuilder->finishElement(event);      

            if (!mpWriter->writeLog(event.data()))
            {      
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "ForkingProxyCseObserver initial event log write failed");
            }
            mpWriter->flush(); // try to ensure that at least the sequence restart gets to the file 
         }
         else
         {
            
         }
      }
   }

   // get my inbound OsMsg queue
   OsMsgQ* myTaskQueue = getMessageQueue();

   // set up periodic timer to flush log file
   OsQueuedEvent* pEvent = new OsQueuedEvent(*myTaskQueue, 0);
   mFlushTimer = new OsTimer(*pEvent) ;
   mFlushTimer->periodicEvery(OsTime(), OsTime(ForkingProxyCallStateFlushInterval, 0)) ;

   // Register to get incoming requests
   sipUserAgent.addMessageObserver(*myTaskQueue,
                                   SIP_INVITE_METHOD, // just INVITEs
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
ForkingProxyCseObserver::~ForkingProxyCseObserver()
{
   if (mpBuilder)
   {
      delete mpBuilder;
      mpBuilder = NULL;
   }
   if (mpWriter)
   {
      mpWriter->flush();
      mpWriter = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean ForkingProxyCseObserver::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();
   switch (msgType)
   {
   case OsMsg::OS_EVENT:
      switch (eventMessage.getMsgSubType())
      {
      case OsEventMsg::NOTIFY:
         if (mpWriter)
         {
            mpWriter->flush();
         }
         break;
      }
      break ;
      
   case OsMsg::PHONE_APP:
   {
      SipMessage* sipRequest;

      if(SipMessageEvent::TRANSPORT_ERROR == ((SipMessageEvent&)eventMessage).getMessageStatus())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "ForkingProxyCseObserver::handleMessage transport error");
      }
      else if((sipRequest = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage()))
      {
         Url toUrl;
         sipRequest->getToUrl(toUrl);
         UtlString toTag;
         toUrl.getFieldParameter("tag", toTag);

         if (toTag.isNull())
         {
            // get the sequence data
            mSequenceNumber++;
            
            OsTime timeNow;
            OsDateTime::getCurTime(timeNow); 

            // get the dialog information
            UtlString contact;
            sipRequest->getContactEntry(0, &contact);

            UtlString callId;
            sipRequest->getCallIdField(&callId);

            // get the To and From header fields
            Url fromUrl;
            sipRequest->getFromUrl(fromUrl);
            UtlString fromTag;
            fromUrl.getFieldParameter("tag", fromTag);

            UtlString toField;
            sipRequest->getToField(&toField);
            
            UtlString fromField;
            sipRequest->getFromField(&fromField);

            // construct the event record
            if (mpBuilder)
            {
               mpBuilder->callRequestEvent(mSequenceNumber, timeNow, contact);
               mpBuilder->addCallData(callId, fromTag, toTag, fromField, toField);

               UtlString via;
               for (int i=0; sipRequest->getViaField(&via, i); i++)
               {
                  mpBuilder->addEventVia(via);
               }
               mpBuilder->completeCallEvent();
            
               // get the complete event record
               UtlString event;
               mpBuilder->finishElement(event);
               
               // write it to the log
               if (mpWriter)
               {
                  mpWriter->writeLog(event.data());
               }               
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "ForkingProxyCseObserver getMessage returned NULL");
      }
   }
   break;
   
   default:
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "ForkingProxyCseObserver invalid message type %d", msgType );
   }
   } // end switch (msgType)
   
   return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

