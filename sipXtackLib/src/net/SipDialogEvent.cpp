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
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Dialog::Dialog(const char* dialogId,
               const char* callId,
               const char* localTag,
               const char* remoteTag,
               const char* direction)
{
   mId = dialogId;
   mCallId = callId;
   mLocalTag = localTag;
   mRemoteTag = remoteTag;
   mDirection = direction;
}


// Destructor
Dialog::~Dialog()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Dialog&
Dialog::operator=(const Dialog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
Dialog::Dialog(const Dialog& rDialog)
{
   mId = rDialog.mId;
   mCallId = rDialog.mCallId;
   mLocalTag = rDialog.mLocalTag;
   mRemoteTag = rDialog.mRemoteTag;
   mDirection = rDialog.mDirection;
}

/* ============================ ACCESSORS ================================= */
void Dialog::getDialog(UtlString& dialogId,
                       UtlString& callId,
                       UtlString& localTag,
                       UtlString& remoteTag,
                       UtlString& direction) const
{
   dialogId = mId;
   callId = mCallId;
   localTag = mLocalTag;
   remoteTag = mRemoteTag;
   direction = mDirection;
}


void Dialog::getCallId(UtlString& callId) const
{
   callId = mCallId;
}


void Dialog::setDialogId(const char* dialogId)
{
   mId = dialogId;
}


void Dialog::getDialogId(UtlString& dialogId) const
{
   dialogId = mId;
}


void Dialog::setState(const char* state,
                      const char* event,
                      const char* code)
{
   mState = state;
   mEvent = event;
   mCode = code;
}


void Dialog::setTags(const char* localTag,
                     const char* remoteTag)
{
   mLocalTag = localTag;
   mRemoteTag = remoteTag;
}


void Dialog::getState(UtlString& state,
                      UtlString& event,
                      UtlString& code) const
{
   state = mState;
   event = mEvent;
   code = mCode;
}


void Dialog::setDuration(const unsigned long duration)
{
   mDuration = duration;
}


unsigned long Dialog::getDuration() const
{
   return mDuration;
}


void Dialog::setReplaces(const char* callId,
                         const char* localTag,
                         const char* remoteTag)
{
   mNewCallId = callId;
   mNewLocalTag = localTag;
   mNewRemoteTag = remoteTag;
}


void Dialog::getReplaces(UtlString& callId,
                         UtlString& localTag,
                         UtlString& remoteTag) const
{
   callId = mNewCallId;
   localTag = mNewLocalTag;
   remoteTag = mNewRemoteTag;
}


void Dialog::setReferredBy(const char* url,
                           const char* display)
{
   mReferredBy = url;
   mDisplay = display;
}


void Dialog::getReferredBy(UtlString& url,
                           UtlString& display) const
{
   url = mReferredBy;
   display = mDisplay;
}


void Dialog::setLocalIdentity(const char* identity,
                              const char* display)
{
   mLocalIdentity = identity;
   mLocalDisplay = display;
}


void Dialog::getLocalIdentity(UtlString& identity,
                              UtlString& display) const
{
   identity = mLocalIdentity;
   display = mLocalDisplay;
}


void Dialog::setRemoteIdentity(const char* identity,
                               const char* display)
{
   mRemoteIdentity = identity;
   mRemoteDisplay = display;
}


void Dialog::getRemoteIdentity(UtlString& identity,
                               UtlString& display) const
{
   identity = mRemoteIdentity;
   display = mRemoteDisplay;
}


void Dialog::setLocalTarget(const char* url)
{
   mLocalTarget = url;
}


void Dialog::getLocalTarget(UtlString& url) const
{
   url = mLocalTarget;
}


void Dialog::setRemoteTarget(const char* url)
{
   mRemoteTarget = url;
}


void Dialog::getRemoteTarget(UtlString& url) const
{
   url = mRemoteTarget;
}


int Dialog::compareTo(const UtlContainable *b) const
{
   return mCallId.compareTo(((Dialog *)b)->mCallId);
}


unsigned int Dialog::hash() const
{
    return mCallId.hash();
}


static UtlContainableType DB_ENTRY_TYPE = "DialogEvent";

const UtlContainableType Dialog::getContainableType() const
{
    return DB_ENTRY_TYPE;
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipDialogEvent::SipDialogEvent(const char* state, const char* entity)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(DIALOG_EVENT_CONTENT_TYPE);

   mVersion = 0;
   mDialogState = state;
   mEntity = entity;
}

SipDialogEvent::SipDialogEvent(const char* bodyBytes)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(DIALOG_EVENT_CONTENT_TYPE);

   if(bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
   }
   
   ((SipDialogEvent*)this)->mBody = bodyBytes;   
}


// Destructor
SipDialogEvent::~SipDialogEvent()
{
   // Clean up all the dialog elements
   if (!mDialogs.isEmpty())
   {
      mDialogs.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */

void SipDialogEvent::parseBody(const char* bodyBytes)
{
   bool foundDialogs = false;

   if(bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::parseBody incoming package = %s\n", 
                    bodyBytes);
                    
      TiXmlDocument doc("dialogEvent.xml");
      
      doc.Parse(bodyBytes);
      if (!doc.Error())
      {
         TiXmlNode * rootNode = doc.FirstChild ("dialog-info");
        
         TiXmlElement* ucElement = 0;

         if (rootNode != NULL)
         {
            ucElement = rootNode->ToElement();
            
            if (ucElement)
            {
               mVersion = atoi(ucElement->Attribute("version"));
               mDialogState = ucElement->Attribute("state");
               mEntity = ucElement->Attribute("entity");
            }
    
            // Parse each dialog
            for (TiXmlNode *groupNode = rootNode->FirstChild("dialog");
                 groupNode; 
                 groupNode = groupNode->NextSibling("dialog"))
            {
               UtlString dialogId, callId, localTag, remoteTag, direction;
               
               foundDialogs = true;

               // Get the attributes in dialog
               ucElement = groupNode->ToElement();
               if (ucElement)
               {
                  dialogId = ucElement->Attribute("id");
                  callId = ucElement->Attribute("call-id");
                  localTag = ucElement->Attribute("local-tag");
                  remoteTag = ucElement->Attribute("remote-tag");
                  direction = ucElement->Attribute("direction");
               }
                  
               Dialog* pDialog = new Dialog(dialogId, callId, localTag, remoteTag, direction);
               
               // Get the state element
               UtlString state, event, code;
               state = (groupNode->FirstChild("state"))->FirstChild()->Value();
               
               ucElement = groupNode->FirstChild("state")->ToElement();
               if (ucElement)
               {
                  event = ucElement->Attribute("event");
                  code = ucElement->Attribute("code");               
                  pDialog->setState(state, event, code);
               }

               // Get the duration element
               UtlString duration;
               TiXmlNode *subNode = groupNode->FirstChild("duration");
               if (subNode)
               {
                  duration = subNode->FirstChild()->Value();
                  pDialog->setDuration((unsigned long)atoi(duration.data()));
               }
               else
               {
                  pDialog->setDuration(0);
               }
                  

               // Get the local element
               UtlString identity, display, target;
               subNode = groupNode->FirstChild("local");
               if (subNode)
               {
                  TiXmlNode *subNode1 = subNode->FirstChild("identity");
                  if (subNode1)
                  {
                     identity = subNode1->FirstChild()->Value();
                     ucElement = subNode1->ToElement();
                     if (ucElement)
                     {                  
                        display = ucElement->Attribute("display");
                        pDialog->setLocalIdentity(identity, display);
                     }
                  }
                  
                  ucElement = subNode->FirstChild("target")->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setLocalTarget(target);
                  }
               }
               
               // Get the remote element
               subNode = groupNode->FirstChild("remote");
               if (subNode)
               {
                  TiXmlNode *subNode1 = subNode->FirstChild("identity");
                  if (subNode1)
                  {
                     identity = subNode1->FirstChild()->Value();
                     ucElement = subNode1->ToElement();
                     if (ucElement)
                     {
                        display = ucElement->Attribute("display");
                        pDialog->setRemoteIdentity(identity, display);
                     }
                  }
                  
                  ucElement = subNode->FirstChild("target")->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setRemoteTarget(target);      
                  }
               }
             
               // Insert it into the list
               insertDialog(pDialog);               
            }
            if (foundDialogs == false)
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::parseBody no dialogs found");
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::parseBody <dialog-info> not found");
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::parseBody xml parsing error");
      }
   }
}


// Assignment operator
SipDialogEvent&
SipDialogEvent::operator=(const SipDialogEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* ============================ ACCESSORS ================================= */
void SipDialogEvent::insertDialog(Dialog* dialog)
{
   mLock.acquire();
   if (mDialogs.insert(dialog) != NULL)   
   {                 
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::insertDialog Dialog = %p", 
                    dialog);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::insertDialog Dialog = %p failed", 
                    dialog);
   }
   mLock.release();
}


Dialog* SipDialogEvent::removeDialog(Dialog* dialog)
{
   mLock.acquire();
   UtlContainable *foundValue;
   foundValue = mDialogs.remove(dialog);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::removeDialog Dialog = %p", 
                 foundValue);                 

   mLock.release();
   return (Dialog *) foundValue;
}


Dialog* SipDialogEvent::getDialog(UtlString& callId)
{
   mLock.acquire();
   UtlHashMapIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   UtlString foundValue;
   while (pDialog = (Dialog *) dialogIterator())
   {
      pDialog->getCallId(foundValue);
      
      if (foundValue.compareTo(callId) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::getDialog found Dialog = %p for callId %s", 
                       pDialog, callId.data());                 
            
         mLock.release();
         return pDialog;
      }
   }     
          
   OsSysLog::add(FAC_SIP, PRI_WARNING, "SipDialogEvent::getDialog could not found the Dialog for callId = %s", 
                 callId.data());                 
            
   mLock.release();
   return NULL;
}


UtlBoolean SipDialogEvent::isEmpty()
{
   return (mDialogs.isEmpty());
}


int SipDialogEvent::getLength() const
{
   int length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}


Dialog* SipDialogEvent::getFirstDialog()
{
   mLock.acquire();
   UtlHashMapIterator dialogIterator(mDialogs);
   Dialog* pDialog = (Dialog *) dialogIterator();
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::getFirstDialog Dialog = %p", 
                 pDialog);                 
            
   mLock.release();
   return pDialog;
}


void SipDialogEvent::getAllDialogs(UtlHashMap& dialogs)
{
   mLock.acquire();
   mDialogs.copyInto(dialogs);
   mLock.release();
}


void SipDialogEvent::buildBody() const
{
   UtlString dialogEvent;
   UtlString singleLine;
   char version[20];
   char durationBuffer[20];

   // Construct the xml document of dialog event
   dialogEvent = UtlString(XML_VERSION_1_0);

   // Dialog Information Structure
   dialogEvent.append(BEGIN_DIALOG_INFO);

   Url entityUri(mEntity);
   sprintf(version, "%d", mVersion);

   dialogEvent.append(VERSION_EQUAL);
   singleLine = DOUBLE_QUOTE + UtlString(version) + DOUBLE_QUOTE;
   dialogEvent += singleLine;
   
   dialogEvent.append(STATE_EQUAL);
   singleLine = DOUBLE_QUOTE + mDialogState + DOUBLE_QUOTE;
   dialogEvent += singleLine;

   dialogEvent.append(ENTITY_EQUAL);
   singleLine = DOUBLE_QUOTE + entityUri.toString() + DOUBLE_QUOTE;
   dialogEvent += singleLine;
   dialogEvent.append(END_LINE);
 
   // Dialog elements
   ((SipDialogEvent*)this)->mLock.acquire();
   UtlHashMapIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   while (pDialog = (Dialog *) dialogIterator())
   {
      UtlString id, callId, localTag, remoteTag, direction;
      pDialog->getDialog(id, callId, localTag, remoteTag, direction);

      dialogEvent.append(BEGIN_DIALOG);
      singleLine = DOUBLE_QUOTE + id + DOUBLE_QUOTE;
      dialogEvent += singleLine;
      if (!callId.isNull())
      {
         dialogEvent.append(CALL_ID_EQUAL);
         singleLine = DOUBLE_QUOTE + callId + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!localTag.isNull())
      {
         dialogEvent.append(LOCAL_TAG_EQUAL);
         singleLine = DOUBLE_QUOTE + localTag + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!remoteTag.isNull())
      {
         dialogEvent.append(REMOTE_TAG_EQUAL);
         singleLine = DOUBLE_QUOTE + remoteTag + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }
   
      if (!direction.isNull())
      {
         dialogEvent.append(DIRECTION_EQUAL);
         singleLine = DOUBLE_QUOTE + direction + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }
      dialogEvent.append(END_LINE);

      // State element
      UtlString state, event, code;
      pDialog->getState(state, event, code);

      dialogEvent.append(BEGIN_STATE);
      if (!event.isNull())
      {
         dialogEvent.append(EVENT_EQUAL);
         singleLine = DOUBLE_QUOTE + event + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!code.isNull())
      {
         dialogEvent.append(CODE_EQUAL);
         singleLine = DOUBLE_QUOTE + code + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      // End of state element
      singleLine = END_BRACKET + state + END_STATE;
      dialogEvent += singleLine;

      // Duration element
      int duration = pDialog->getDuration();      
      if (duration !=0)
      {
         duration = OsDateTime::getSecsSinceEpoch() - pDialog->getDuration();
         sprintf(durationBuffer, "%d", duration);
         dialogEvent += BEGIN_DURATION + UtlString(durationBuffer) + END_DURATION;     
      }
      
      // Local element
      UtlString identity, displayName, target;
      pDialog->getLocalIdentity(identity, displayName);
      pDialog->getLocalTarget(target);

      dialogEvent.append(BEGIN_LOCAL);
      if (!identity.isNull())
      {
         dialogEvent.append(BEGIN_IDENTITY);
         if (!displayName.isNull())
         {
            NameValueTokenizer::frontBackTrim(&displayName, "\"");
            dialogEvent.append(DISPLAY_EQUAL);
            singleLine = DOUBLE_QUOTE + displayName + DOUBLE_QUOTE;
            dialogEvent += singleLine;
         }
         
         singleLine = END_BRACKET + identity + END_IDENTITY;
         dialogEvent += singleLine;
      }

      if (!target.isNull() && target.compareTo("sip:") != 0)
      {
         singleLine = BEGIN_TARTGET + target + END_TARGET;
         dialogEvent += singleLine;
      }

      // End of local element
      dialogEvent.append(END_LOCAL);

      // Remote element
      pDialog->getRemoteIdentity(identity, displayName);
      pDialog->getRemoteTarget(target);

      dialogEvent.append(BEGIN_REMOTE);
      if (!identity.isNull())
      {
         dialogEvent.append(BEGIN_IDENTITY);
         if (!displayName.isNull())
         {
            NameValueTokenizer::frontBackTrim(&displayName, "\"");
            dialogEvent.append(DISPLAY_EQUAL);
            singleLine = DOUBLE_QUOTE + displayName + DOUBLE_QUOTE;
            dialogEvent += singleLine;
         }
   
         singleLine = END_BRACKET + identity + END_IDENTITY;
         dialogEvent += singleLine;
      }
      
      if (!target.isNull() && target.compareTo("sip:") != 0)
      {
         singleLine = BEGIN_TARTGET + target + END_TARGET;
         dialogEvent += singleLine;
      }

      // End of remote element
      dialogEvent.append(END_REMOTE);  

      // End of dialog element
      dialogEvent.append(END_DIALOG);  
   }

   // End of dialog-info element
   dialogEvent.append(END_DIALOG_INFO);  
   
   ((SipDialogEvent*)this)->mLock.release();
  
   ((SipDialogEvent*)this)->mBody = dialogEvent;
   ((SipDialogEvent*)this)->bodyLength = dialogEvent.length();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::buildBody Dialog content = \n%s", 
                 dialogEvent.data());
  
   // Increment the version number
   ((SipDialogEvent*)this)->mVersion++;
}


void SipDialogEvent::getBytes(const char** bytes, int* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);
   ((SipDialogEvent*)this)->mBody = tempBody.data();

   *bytes = mBody.data();
}


void SipDialogEvent::getBytes(UtlString* bytes, int* length) const
{
   buildBody();
   
   *bytes = ((SipDialogEvent*)this)->mBody;
   *length = ((SipDialogEvent*)this)->bodyLength;
}


void SipDialogEvent::setEntity(const char* entity)
{
   mEntity = entity;
}


void SipDialogEvent::getEntity(UtlString& entity) const
{
   entity = mEntity;
}


void SipDialogEvent::setState(const char* state)
{
   mDialogState = state;
}


void SipDialogEvent::getState(UtlString& state) const
{
   state = mDialogState;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

