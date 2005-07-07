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
#include <os/OsWriteLock.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/SipPresenceEvent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Tuple::Tuple(const char* tupleId)
{
   mId = tupleId;
}


// Destructor
Tuple::~Tuple()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Tuple&
Tuple::operator=(const Tuple& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
Tuple::Tuple(const Tuple& rTuple)
{
   mId = rTuple.mId;
}

/* ============================ ACCESSORS ================================= */
void Tuple::setTupleId(const char* tupleId)
{
   mId = tupleId;
}


void Tuple::getTupleId(UtlString& tupleId) const
{
   tupleId = mId;
}


void Tuple::setStatus(const char* status)
{
   mStatus = status;
}


void Tuple::getStatus(UtlString& status) const
{
   status = mStatus;
}


void Tuple::setContact(const char* contactUrl,
                       const float priority)
{
   mContactUrl = contactUrl;
   mPriority = priority;
}


void Tuple::getContact(UtlString& contactUrl,
                       float& priority) const
{
   contactUrl = mContactUrl;
   priority = mPriority;
}


int Tuple::compareTo(const UtlContainable *b) const
{
   return mId.compareTo(((Tuple *)b)->mId);
}


unsigned int Tuple::hash() const
{
    return mId.hash();
}


static UtlContainableType DB_ENTRY_TYPE = "PresenceEvent";

const UtlContainableType Tuple::getContainableType() const
{
    return DB_ENTRY_TYPE;
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipPresenceEvent::SipPresenceEvent(const char* entity, const char*bodyBytes)
   : mRWMutex(OsRWMutex::Q_PRIORITY)
{
   remove(0);
   append(PRESENCE_EVENT_CONTENT_TYPE);

   mEntity = entity;
   
   if(bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
  
      ((SipPresenceEvent*)this)->mBody = bodyBytes;
   }
}


// Destructor
SipPresenceEvent::~SipPresenceEvent()
{
   // Clean up all the tuple elements
   if (mTuples.isEmpty())
   {
      mTuples.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */

void SipPresenceEvent::parseBody(const char* bodyBytes)
{
   if(bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::parseBody incoming package = %s\n", 
                    bodyBytes);
                    
      TiXmlDocument doc("PresenceEvent.xml");
      
      if (doc.Parse(bodyBytes))
      {
         TiXmlNode * rootNode = doc.FirstChild ("presence");
        
         TiXmlElement* ucElement = 0;

         if (rootNode != NULL)
         {
            ucElement = rootNode->ToElement();
            
            if (ucElement)
            {
               mEntity = ucElement->Attribute("entity");
            }
      
            // Parse each tuple
            for (TiXmlNode *groupNode = rootNode->FirstChild("tuple");
                 groupNode; 
                 groupNode = groupNode->NextSibling("tuple"))
            {
               UtlString tupleId;
               
               // Get the attributes in tuple
               ucElement = groupNode->ToElement();
               if (ucElement)
               {
                  tupleId = ucElement->Attribute("id");
               }
                  
               Tuple* pTuple = new Tuple(tupleId);
               
               // Get the status element
               UtlString status;
               status = ((groupNode->FirstChild("status"))->FirstChild("basic"))->FirstChild()->Value();
               pTuple->setStatus(status);

               // Get the contact element
               UtlString contact, priority;
               TiXmlNode *subNode = groupNode->FirstChild("contact");
               if (subNode)
               {
                  contact = subNode->FirstChild()->Value();
                  ucElement = subNode->ToElement();
               
                  if (ucElement)
                  {                  
                     priority = ucElement->Attribute("priority");
                  }
                  
               pTuple->setContact(contact, atof(priority));
               }
             
               // Insert it into the list
               insertTuple(pTuple);               
            }
         }
      }
   }
}


// Assignment operator
SipPresenceEvent&
SipPresenceEvent::operator=(const SipPresenceEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void SipPresenceEvent::insertTuple(Tuple* tuple)
{
   OsWriteLock lock(mRWMutex);
   if (mTuples.insert(tuple) != NULL)   
   {                 
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::insertTuple Tuple = %p", 
                    tuple);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceEvent::insertTuple Tuple = %p failed", 
                    tuple);
   }
}


Tuple* SipPresenceEvent::removeTuple(Tuple* tuple)
{
   OsWriteLock lock(mRWMutex);
   UtlContainable *foundValue;
   foundValue = mTuples.remove(tuple);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::removeTuple Tuple = %p", 
                 foundValue);                 

   return (Tuple *) foundValue;
}


Tuple* SipPresenceEvent::getTuple(UtlString& tupleId)
{
   OsWriteLock lock(mRWMutex);
   UtlHashMapIterator tupleIterator(mTuples);
   Tuple* pTuple;
   UtlString foundValue;
   while (pTuple = (Tuple *) tupleIterator())
   {
      pTuple->getTupleId(foundValue);
      
      if (foundValue.compareTo(tupleId) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::getTuple found Tuple = %p for tupleId %s", 
                       pTuple, tupleId.data());                 
            
         return pTuple;
      }
   }     
          
   OsSysLog::add(FAC_SIP, PRI_WARNING, "SipPresenceEvent::getTuple could not found the Tuple for tupleId = %s", 
                 tupleId.data());                 
            
   return NULL;
}

UtlBoolean SipPresenceEvent::isEmpty()
{
   OsWriteLock lock(mRWMutex);
   return (mTuples.isEmpty());
}

int SipPresenceEvent::getLength() const
{
   int length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}

void SipPresenceEvent::buildBody() const
{
   UtlString PresenceEvent;
   char PresenceEventBuffer[MAX_CHAR_SIZE];
   char tempBuffer[MAX_CHAR_SIZE];

   // Construct the xml document of Tuple event
   PresenceEvent = UtlString(XML_VERSION_1_0);

   // Tuple Information Structure
   sprintf(PresenceEventBuffer, "<presence xmlns=\"%s\" entity=\"%s\">\n",
           PRESENCE_XMLNS, mEntity.data());
   PresenceEvent += UtlString(PresenceEventBuffer);
 
   // Tuple elements
   OsWriteLock lock(((SipPresenceEvent*)this)->mRWMutex);
   UtlHashMapIterator tupleIterator(mTuples);
   Tuple* pTuple;
   while (pTuple = (Tuple *) tupleIterator())
   {
      
      UtlString tupleId;
      pTuple->getTupleId(tupleId);

      sprintf(PresenceEventBuffer, "<tuple id=\"%s\">\n", tupleId.data());
      PresenceEvent += UtlString(PresenceEventBuffer);

      // Status element
      UtlString status;
      pTuple->getStatus(status);
      sprintf(PresenceEventBuffer, "<status>\n<basic>");
      sprintf(tempBuffer, "%s</basic>\n</status>\n", status.data());
      strcat(PresenceEventBuffer, tempBuffer);
      PresenceEvent += UtlString(PresenceEventBuffer);
      
      // Contact element
      UtlString contact;
      float priority;
      pTuple->getContact(contact, priority);
      if (!contact.isNull())
      {
         sprintf(PresenceEventBuffer, "<contact");
         if (priority != 0.0)
         {
            sprintf(tempBuffer, " priority=\"%.3f\">%s</contact>\n", priority, contact.data());
         }
         else
         {
            sprintf(tempBuffer, ">%s</contact>\n", contact.data());
         }
         strcat(PresenceEventBuffer, tempBuffer);
         PresenceEvent += UtlString(PresenceEventBuffer);
      }

      // End of Tuple element
      PresenceEvent += UtlString("</tuple>\n");
   }

   // End of Tuple-info element
   PresenceEvent += UtlString("</presence>\n");

   ((SipPresenceEvent*)this)->mBody = PresenceEvent;
   ((SipPresenceEvent*)this)->bodyLength = PresenceEvent.length();
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTupleEvent::getBytes Tuple content = \n%s", 
                 PresenceEvent.data());                 
}

void SipPresenceEvent::getBytes(const char** bytes, int* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);
   ((SipPresenceEvent*)this)->mBody = tempBody.data();

   *bytes = mBody.data();
}

void SipPresenceEvent::getBytes(UtlString* bytes, int* length) const
{
   buildBody();
   
   *bytes = ((SipPresenceEvent*)this)->mBody;
   *length = ((SipPresenceEvent*)this)->bodyLength;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

