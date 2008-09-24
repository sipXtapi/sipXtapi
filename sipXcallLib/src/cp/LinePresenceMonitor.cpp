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

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlSListIterator.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>
#include <net/XmlRpcRequest.h>
#include <cp/LinePresenceMonitor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_REFRESH_INTERVAL      180000
#define CONFIG_ETC_DIR                SIPX_CONFDIR

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LinePresenceMonitor::LinePresenceMonitor(int userAgentPort,
                                         UtlString& domainName,
                                         UtlString& groupName,
                                         bool local,
                                         Url& remoteServer)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Bind the SIP user agent to a port and start it up
   mpUserAgent = new SipUserAgent(userAgentPort, userAgentPort);
   mpUserAgent->start();
   
   mGroupName = groupName;
   mLocal = local;

   if (mLocal)
   {
      // Create a local Sip Dialog Monitor
      mpDialogMonitor = new SipDialogMonitor(mpUserAgent,
                                             domainName,
                                             userAgentPort,
                                             DEFAULT_REFRESH_INTERVAL,
                                             false);
      
      // Add itself to the dialog monitor for state change notification
      mpDialogMonitor->addStateChangeNotifier("Line_Presence_Monitor", this);

      // Create a local Sip Presence Monitor
      UtlString configFile(CONFIG_ETC_DIR);
      configFile.append("/sipxpresence.xml");
      mpPresenceMonitor = new SipPresenceMonitor(mpUserAgent,
                                                 domainName,
                                                 userAgentPort,
                                                 configFile,
                                                 false);
      
      // Add itself to the presence monitor for state change notification
      mpPresenceMonitor->addStateChangeNotifier("Line_Presence_Monitor", this);
   }
   else
   {
      // Create the SIP Subscribe Client
      mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager); // Component for refreshing the subscription
      mpRefreshMgr->start();
   
      mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager, *mpRefreshMgr);
      mpSipSubscribeClient->start();
      
      mRemoteServer = remoteServer;
   }
}


// Destructor
LinePresenceMonitor::~LinePresenceMonitor()
{
   // Shut down the sipUserAgent
   mpUserAgent->shutdown(FALSE);

   while(!mpUserAgent->isShutdownDone())
   {
      ;
   }

   delete mpUserAgent;
   
   if (mpDialogMonitor)
   {
      // Remove itself to the dialog monitor
      mpDialogMonitor->removeStateChangeNotifier("Line_Presence_Monitor");

      delete mpDialogMonitor;
   }
   
   if (!mSubscribeList.isEmpty())
   {
      mSubscribeList.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
LinePresenceMonitor&
LinePresenceMonitor::operator=(const LinePresenceMonitor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */
bool LinePresenceMonitor::setStatus(const Url& aor, const Status value)
{
   bool result = false;
   
   mLock.acquire();
   UtlString contact = aor.toString();
   UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mSubscribeList.findValue(&contact));
   LinePresenceBase* line = (LinePresenceBase *) container->getValue();
   if (line)
   {
      // Set the state value in LinePresenceBase
      switch (value)
      {
      case StateChangeNotifier::ON_HOOK:
         if (!line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, true);
            result = true;
         }
         
         break;
         
      case StateChangeNotifier::OFF_HOOK:
         if (line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, false);
            result = true;
         }
         
         break;

      case StateChangeNotifier::RINGING:
         if (line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, false);
            result = true;
         }
         
         break;

      case StateChangeNotifier::PRESENT:
         if (!line->getState(LinePresenceBase::PRESENT))
         {
            line->updateState(LinePresenceBase::PRESENT, true);
            result = true;
         }
         
         break;
         
      case StateChangeNotifier::AWAY:
         if (line->getState(LinePresenceBase::PRESENT))
         {
            line->updateState(LinePresenceBase::PRESENT, false);
            result = false;
         }
         
         break;
         
      default:
      
         OsSysLog::add(FAC_SIP, PRI_ERR, "LinePresenceMonitor::setStatus tried to set an unsupported value %d",
                       value);
      }
   }
      
   mLock.release();
   
   return result;
}

OsStatus LinePresenceMonitor::subscribe(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   Url* lineUrl = line->getUri();
   
   if (mLocal)
   {
      if(mpDialogMonitor->addExtension(mGroupName, *lineUrl))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "addExtension");
      
      request.addParam(&mGroupName);
      UtlString contact = lineUrl->toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      if (request.execute(response))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }      
   }
   
   // Insert the line to the Subscribe Map
   mSubscribeList.insertKeyAndValue(new UtlString(lineUrl->toString()),
                                   new UtlVoidPtr(line));

   mLock.release();
   
   return result;
}

OsStatus LinePresenceMonitor::unsubscribe(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   Url* lineUrl = line->getUri();
   
   if (mLocal)
   {
      if (mpDialogMonitor->removeExtension(mGroupName, *lineUrl))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "removeExtension");
      
      request.addParam(&mGroupName);
      UtlString contact = lineUrl->toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      if (request.execute(response))
      {
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_FAILED;
      }      
   }
      
   // Remove the line from the Subscribe Map
   UtlString contact(lineUrl->toString());
   mSubscribeList.destroy(&contact);
   
   mLock.release();
   return result;
}

OsStatus LinePresenceMonitor::subscribe(UtlSList& list)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      subscribe(line);
   }

   mLock.release();
   
   return result;
}

OsStatus LinePresenceMonitor::unsubscribe(UtlSList& list)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      unsubscribe(line);
   }

   mLock.release();
   
   return result;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

