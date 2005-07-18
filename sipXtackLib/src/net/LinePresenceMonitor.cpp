// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/LinePresenceMonitor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_REFRESH_INTERVAL      180000
#define DEFAULT_GROUP_NAME   "acd@pingtel.com"

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LinePresenceMonitor::LinePresenceMonitor(int userAgentPort,
                                         UtlString& domainName)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Bind the SIP user agent to a port and start it up
   mpUserAgent = new SipUserAgent(userAgentPort, userAgentPort);
   mpUserAgent->start();

   // Create the Sip Dialog Monitor
   mpDialogMonitor = new SipDialogMonitor(mpUserAgent,
                                          domainName,
                                          userAgentPort,
                                          DEFAULT_REFRESH_INTERVAL);
   
   // Add itself to the dialog monitor for state change notification
   mpDialogMonitor->addStateChangeNotifier("Line_Presence_Monitor", this);
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
      delete mpDialogMonitor;
   }
   
   if (!mSubscribeMap.isEmpty())
   {
      mSubscribeMap.destroyAll();
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
   LinePresenceBase* line = (LinePresenceBase *) mSubscribeMap.findValue(&contact);
   if (line)
   {
      // Set the state value in LinePresenceBase
      if (value == StateChangeNotifier::ON_HOOK)
      {
         if (!line->getState(LinePresenceBase::ON_HOOK))
         {
            line->updateState(LinePresenceBase::ON_HOOK, true);
            result = true;
         }
      }
      else
      {
         if (value == StateChangeNotifier::OFF_HOOK)
         {
            if (line->getState(LinePresenceBase::ON_HOOK))
            {
               line->updateState(LinePresenceBase::ON_HOOK, false);
               result = true;
            }
         }
         else
         {
            if (value == StateChangeNotifier::RINGING)
            {
               if (line->getState(LinePresenceBase::ON_HOOK))
               {
                  line->updateState(LinePresenceBase::ON_HOOK, false);
                  result = true;
               }
            }
            else
            {
               if (value == StateChangeNotifier::PRESENT)
               {
                  if (!line->getState(LinePresenceBase::PRESENT))
                  {
                     line->updateState(LinePresenceBase::PRESENT, true);
                     result = true;
                  }
               }
               else
               {
                  if (value == StateChangeNotifier::AWAY)
                  {
                     if (line->getState(LinePresenceBase::PRESENT))
                     {
                        line->updateState(LinePresenceBase::PRESENT, false);
                        result = false;
                     }
                  }                  
               }
            }
         }
      }      
   }
      
   mLock.release();
   
   return result;
}

OsStatus LinePresenceMonitor::subscribe(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   Url* lineUrl = line->getAddress();
   UtlString groupName(DEFAULT_GROUP_NAME);
   
   if(mpDialogMonitor->addExtension(groupName, *lineUrl))
   {
      result = OS_SUCCESS;
   }
   else
   {
      result = OS_FAILED;
   }
   
   // Insert the line to the Subscribe Map
   mSubscribeMap.insertKeyAndValue(new UtlString(lineUrl->toString()),
                                   (UtlVoidPtr *) line);

   mLock.release();
   
   return result;
}

OsStatus LinePresenceMonitor::unsubscribe(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;
   mLock.acquire();
   Url* lineUrl = line->getAddress();
   UtlString groupName(DEFAULT_GROUP_NAME);
   
   if (mpDialogMonitor->removeExtension(groupName, *lineUrl))
   {
      result = OS_SUCCESS;
   }
   else
   {
      result = OS_FAILED;
   }
      
   // Insert the line to the Subscribe Map
   UtlString contact(lineUrl->toString());
   mSubscribeMap.destroy(&contact);
   
   mLock.release();
   return result;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

