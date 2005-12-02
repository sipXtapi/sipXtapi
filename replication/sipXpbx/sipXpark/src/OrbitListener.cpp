// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "OrbitListener.h"
#include "ParkedCallObject.h"
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>
#include <os/OsFS.h>
#include "xmlparser/tinyxml.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6

#define DEFAULT_MOH_AUDIO                  "file://"SIPX_PARKMUSICDIR"/default.wav"

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitListener::OrbitListener(CallManager* callManager)
{
   mpCallManager = callManager;
}


//Destructor
OrbitListener::~OrbitListener()
{
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean OrbitListener::handleMessage(OsMsg& rMsg)
{
   // React to telephony events
   if(rMsg.getMsgSubType()== TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      int taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());
      TaoString arg(argList, TAOMESSAGE_DELIMITER);

#ifdef DEBUGGING
      dumpTaoMessageArgs(taoEventId, arg) ;
#endif        
      UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
      UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;

      switch (taoEventId) 
      {
         case PtEvent::CONNECTION_OFFERED:
            OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call arrived: callId: %s, address: %s\n", 
                          callId.data(), address.data());

            mpCallManager->acceptConnection(callId, address);
            mpCallManager->answerTerminalConnection(callId, address, "*");

            break;


         case PtEvent::CONNECTION_ESTABLISHED:
            if (localConnection) 
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call connected: callId %s, address: %s",
                             callId.data(), address.data());

               ParkedCallObject* pThisCall = NULL;
               UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
               
               if (container != NULL)
               {
                  pThisCall = (ParkedCallObject *) container->getValue();
               }
               
               if (pThisCall == NULL)
               {
                  //
                  // New call, validate.
                  //
                  UtlString audio;
                  if (validateOrbitRequest(callId, address, audio) == OS_SUCCESS)
                  {
                     pThisCall = new ParkedCallObject(mpCallManager, callId, audio);
                     pThisCall->setAddress(address);

                     // Create a player and start to play out the file
                     if (pThisCall->playAudio() == OS_SUCCESS)
                     {
                        // Put it in a list
                        mCalls.insertKeyAndValue(new UtlString(callId), new UtlVoidPtr(pThisCall));
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage insert call object %p to the list",
                                      pThisCall);
                     }
                     else
                     {
                        // Drop the call
                        mpCallManager->drop(callId);
                        OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to failure of playing audio",
                                      callId.data());

                        delete pThisCall;
                     }
                  }
                  else
                  {
                     // Do not accept the call to the requested orbit address.
                     mpCallManager->drop(callId);
                     OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Drop callId %s due to invalid orbit address",
                                   callId.data());
                  }
               }
               else
               {
                  //
                  // Second leg of transfer.
                  //
                  OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - change the call address from %s to %s",
                                pThisCall->getAddress().data(), address.data());
                  pThisCall->setAddress(address);
               }
            }

            break;
            

         case PtEvent::CONNECTION_DISCONNECTED:
            if (!localConnection)
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - Call Dropped: callId %s, address: %s",
                             callId.data(), address.data());

               // See if the callId is in our list and if the address matches.
               UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mCalls.findValue(&callId));
               ParkedCallObject* pDroppedCall = (ParkedCallObject *) container->getValue();
               OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - found call object %p for %s\n",
                             pDroppedCall, callId.data());
               if (pDroppedCall == NULL)
               {
                  OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::handleMessage - No callId %s founded in the active call list\n",
                                callId.data());
               }
               else if (address.compareTo(pDroppedCall->getAddress()) == 0)
               {
                  // Remove the call from the pool and clean up the call
                  mCalls.destroy(&callId);
   
                  pDroppedCall->cleanUp();
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "OrbitListener::handleMessage remove call object %p from the list",
                                pDroppedCall);
                  delete pDroppedCall;
   
                  // Drop the call
                  mpCallManager->drop(callId);
               }
            }

            break;
            

         case PtEvent::CONNECTION_FAILED:
            OsSysLog::add(FAC_ACD, PRI_WARNING, "OrbitListener::handleMessage - Dropping call: %s\n", callId.data());

            mpCallManager->drop(callId);

            break;
      }
   }
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


unsigned int OrbitListener::validateOrbitRequest(UtlString& callId, UtlString& address, UtlString& audio)
{
   OsPath workingDirectory;
   UtlString fromField, userId;

   mpCallManager->getFromField(callId.data(), address.data(), fromField);
   Url orbitUrl(fromField);
   orbitUrl.getUserId(userId);

   audio = "";

   //
   // Now scan the orbit.xml file looking for a matching orbit entry
   //

   // First try and find the orbit.xml file, either in the SIPX_CONFDIR or the CWD.
   if (OsFileSystem::exists(SIPX_CONFDIR))
   {
      workingDirectory = SIPX_CONFDIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);

   } else
   {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
   }

   UtlString fileName = workingDirectory + OsPathBase::separator + ORBIT_CONFIG_FILE;

   TiXmlDocument doc(fileName);

   if (doc.LoadFile())
   {
      TiXmlNode* rootNode = doc.FirstChild("orbits");
      if (rootNode != NULL)
      {
         // See if this is a music-on-hold call or an orbit call
         if (userId == "moh")
         {
            TiXmlNode *groupNode = rootNode->FirstChild("music-on-hold");
            if (rootNode != NULL)
            {
               TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
               audio = (audioNode->FirstChild())->Value();
            }
         }
         else
         {
            // Search each <orbit> group
            for (TiXmlNode *groupNode = rootNode->FirstChild("orbit");
                 groupNode;
                 groupNode = groupNode->NextSibling("orbit"))
            {
               // See if the <extension> matches
               TiXmlNode* orbitNode = groupNode->FirstChild("extension");
               if ((orbitNode->FirstChild())->Value() == userId)
               {
                  TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
                  audio = (audioNode->FirstChild())->Value();
                  break;
               }
            }
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "OrbitListener::validateOrbitRequest - Unable to open orbit config file: %s", fileName.data());
   }

   if (audio != "")
   {
      return OS_SUCCESS;
   }
   else
   {
      // Check if this is for MOH.  If so, use the default audio file.
      if (userId == "moh")
      {
         audio = DEFAULT_MOH_AUDIO;
         return OS_SUCCESS;
      }
      else
      {
         return OS_FAILED;
      }
   }
}


void OrbitListener::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
   osPrintf("===>\nMessage type: %d args:\n\n", eventId) ;
        
   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      osPrintf("\targ[%d]=\"%s\"\n", argIndex, args[argIndex]);
   }
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

