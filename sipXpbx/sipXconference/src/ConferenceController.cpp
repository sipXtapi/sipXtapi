//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

// APPLICATION INCLUDES
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEventMsg.h>
#include <os/OsSysLog.h>
#include <ConferenceController.h>
#include <ConferenceControllerMsg.h>
#include <Conference.h>
#include <Leg.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <cp/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpIntMessage.h>
#include <ptapi/PtConnection.h>
#include <ptapi/PtCall.h>
#include <ptapi/PtTerminalConnection.h>
#include <tao/TaoProviderAdaptor.h>
#include <tao/TaoListenerEventMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SERVER_QUEUE_SIZE 100

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ConferenceController::ConferenceController(int udpPort,
                                           int tcpPort,
                                           int rtpBase,
                                           int maxSessions,
                                           UtlString& domainHostPort) :
OsServerTask("Call-%d", NULL, DEF_MAX_MSGS, DEF_PRIO, DEF_OPTIONS,
             OsTaskBase::DEF_STACKSIZE),
mSipxInstance(SIPX_INST_NULL),
mDomainHostPort(domainHostPort),
mMutex(OsBSem::Q_FIFO, OsBSem::FULL)
{
   // Construct the sipXtapi user agent instance.
   SIPX_RESULT result = sipxInitialize(&mSipxInstance,
                                       udpPort,
                                       tcpPort,
                                       PORT_NONE,
                                       rtpBase,
                                       maxSessions,
                                       DEFAULT_IDENTITY,
                                       DEFAULT_BIND_ADDRESS);
   if (result != SIPX_RESULT_SUCCESS)
   {
      OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                    "ConferenceController::_ sipxInitialize failed, result = %d",
                    result);
      return;
   }

   // Add the event listener.
   result = sipxListenerAdd(mSipxInstance,
                            eventListener,
                            (void*) this);
   if (result != SIPX_RESULT_SUCCESS)
   {
      OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                    "ConferenceController::_ sipxListenerAdd failed, result = %d",
                    result);
      return;
   }

   // Construct one conference object.
   UtlString user("500");
   mConference = new Conference(this, user);
   // Add it to the hash bag of conferences.
   mConferences.insert(mConference);
}

// Destructor
ConferenceController::~ConferenceController()
{
    if (isStarted())
    {
        waitUntilShutDown();
    }

    // Delete the one conference object.
    delete mConference;

    // Finalize the sipXtapi user agent.
    if (mSipxInstance != SIPX_INST_NULL)
    {
       SIPX_RESULT result = sipxUnInitialize(mSipxInstance);
       if (result != SIPX_RESULT_SUCCESS)
       {
          OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                        "ConferenceController::~ sipxUnInitialize failed, result = %d",
                        result);
       }
    }

}

/* ============================ MANIPULATORS ============================== */

UtlBoolean ConferenceController::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   switch(msgType)
   {
   default:
      OsSysLog::add(FAC_CP, PRI_ERR,
                    "ConferenceController::handleMessage: Unknown message type = %d, subtype = %d\n",
                    msgType, msgSubType);
      break;

   case ConferenceControllerEventMsg::MSG_TYPE:
   {
      // Get the components of the message.
      ConferenceControllerEventMsg *ccem =
         dynamic_cast<ConferenceControllerEventMsg*> (&eventMessage);
      SIPX_CALL hCall = ccem->getCall();
      SIPX_LINE hLine = ccem->getLine();
      SIPX_CALLSTATE_MAJOR eMajor = ccem->getMajor();
      SIPX_CALLSTATE_MINOR eMinor = ccem->getMinor();

      char s[100];
      sipxCallEventToString(eMajor, eMinor, s, sizeof (s));
      OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                    "ConferenceController::handleMessage call %d, event %s, major %d, minor %d",
                    hCall, s, eMajor, eMinor);

      // Acquire the semaphore.
      mMutex.acquire();

      // Dispatch on event type.
      switch (eMajor)
      {
      case NEWCALL:
      {
         // Find the conference for the call.
         UtlSListIterator itor(mConferences);
         Conference* conference;
         while (conference = dynamic_cast<Conference*> (itor()))
         {
            if (conference->mSipxLine == hLine)
            {
               break;
            }
         }
         if (!conference)
         {
            OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                          "ConferenceController::handleMessage Unknown line for event: %d",
                          hLine);
            // If the conference couldn't be found, hang up the call.
            OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                          "ConferenceController::handleMessage sipxCallDestroy");
            SIPX_RESULT result = sipxCallDestroy(hCall);
            if (result != SIPX_RESULT_SUCCESS)
            {
               OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                             "ConferenceController::handleConnected sipxCallDestroy failed, result = %d",
                             result);
               break;
            }
         }

         // Have the conference create a new leg object; add it to
         // the lookup table.
         OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                       "ConferenceController::handleMessage adding call %d",
                       hCall);
         indexAddCall(hCall,
                      conference->handleNewCall(hCall, eMinor));
      }
      break;

      // All call state changes are handled the same way, except for
      // creation, disconnect, and destruction -- pass them to the leg
      // for that call.
      case OFFERING:
      case ALERTING:
      case CONNECTED:
      case AUDIO_EVENT:
      case DIALTONE:
      case REMOTE_OFFERING:
      case REMOTE_ALERTING:
      {
         // Look up the call leg for the call handle.
         Leg* leg = indexLookupCall(hCall);
         if (leg)
         {
            // Pass the event to the call leg.
            leg->processEvent(eMajor, eMinor);
         }
         else
         {
            // Record that the call handle couldn't be found.
            OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                          "ConferenceController::handleMessage event for unknown call handle %d",
                          hCall);
         }
      }
      break;

      case DISCONNECTED:
      {
         // DISCONNECTED events are handled by doing sipxCallDestroy, which
         // causes the call to be cleaned up.  Our call leg will be cleaned
         // up when we receive the inevitable DESTROYED event.
         OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                       "Leg::processEvent sipxCallDestroy");
         SIPX_RESULT result = sipxCallAnswer(hCall);
         if (result != SIPX_RESULT_SUCCESS)
         {
            OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                          "ConferenceController::handleMessage sipxCallDestroy failed for call %d, result = %d",
                          hCall, result);
         }
      }
      break;

      case DESTROYED:
      {
         // Look up the call leg for the call handle.
         Leg* leg = indexLookupCall(hCall);
         if (leg)
         {
            OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                          "ConferenceController::handleMessage destroying call %d",
                          hCall);
            // Remove the leg from the index.
            indexDeleteCall(hCall);
            // Destroy the leg itself, which cleans up all aspects of the call..
            delete leg;
         }
         else
         {
            // Record that the call handle couldn't be found.
            OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                          "ConferenceController::handleMessage event for unknown call handle %d",
                          hCall);
         }
      }
      break;

      default:
         OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                       "ConferenceController::handleMessage unhandled event type = %d",
                       eMajor);
         break;
      }

      // Release the semaphore.
      mMutex.release();
   }
   }
   // We never pass messages back to OsServerTask::handleMessage.
   return(TRUE);
}

void ConferenceController::requestShutdown()
{
    // Pass the shut down request to the OsServerTask shutdown method.
    OsServerTask::requestShutdown();
    yield();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

// Tapi event listener.
void ConferenceController::eventListener(SIPX_CALL hCall,
                                         SIPX_LINE hLine,
                                         SIPX_CALLSTATE_MAJOR eMajor,
                                         SIPX_CALLSTATE_MINOR eMinor,  
                                         void* pUserData)
{
   ConferenceController* cc = (ConferenceController*) pUserData;

   OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                 "ConferenceController::eventListener call %d, line %d, major %d, minor %d",
                 hCall, hLine, eMajor, eMinor);
   // Post the event as a message to the message queue for the conference
   // controller.
   cc->postMessage((OsMsg&)
                   (*new ConferenceControllerEventMsg(hCall, hLine,
                                                      eMajor, eMinor)));
}

void ConferenceController::indexAddCall(SIPX_CALL hCall,
                                        Leg* leg)
{
   // Convert the call handle into a UtlInteger.
   UtlInt* index_p = new UtlInt((int) hCall);
   // Insert the leg into the index.
   mIndex.insertKeyAndValue(index_p, leg);
}

void ConferenceController::indexDeleteCall(SIPX_CALL hCall)
{
   // Convert the call handle into a UtlInteger.
   UtlInt index = hCall;
   // Delete the leg from the index.
   UtlContainable* index_p = mIndex.remove(&index);
   // Free the UtlInt object that is the key.
   delete index_p;
}

Leg* ConferenceController::indexLookupCall(SIPX_CALL hCall)
{
   // Convert the call handle into a UtlInteger.
   UtlInt index = hCall;
   // Look up the call leg object for the handle.
   return dynamic_cast<Leg*> (mIndex.findValue(&index));
}
