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
#include <net/SipResourceList.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Resource::Resource(const char* uri)
{
   mUri = uri;
}


// Destructor
Resource::~Resource()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Resource&
Resource::operator=(const Resource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
Resource::Resource(const Resource& rResource)
{
   mUri = rResource.mUri;
}


/* ============================ ACCESSORS ================================= */
void Resource::getResourceUri(UtlString& uri) const
{
   uri = mUri;
}


void Resource::setName(const char* name)
{
   mName = name;
}


void Resource::getName(UtlString& name) const
{
   name = mName;
}


void Resource::setInstance(const char* id,
                           const char* state)
{
   mState = state;
   mId = id;
}


void Resource::getInstance(UtlString& id,
                           UtlString& state) const
{
   state = mState;
   id = mId;
}


int Resource::compareTo(const UtlContainable *b) const
{
   return mUri.compareTo(((Resource *)b)->mUri);
}


unsigned int Resource::hash() const
{
    return mUri.hash();
}


static UtlContainableType DB_ENTRY_TYPE = "ResourceList";

const UtlContainableType Resource::getContainableType() const
{
    return DB_ENTRY_TYPE;
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipResourceList::SipResourceList(const UtlBoolean state,
                                 const char* uri,
                                 const char* type)
   : mRWMutex(OsRWMutex::Q_PRIORITY)
{
   remove(0);
   append(RESOURCE_LIST_CONTENT_TYPE);

   mVersion = 0;
   if (state)
   {
      mFullState = "true";
   }
   else
   {
      mFullState = "false";
   }
   
   mListUri = uri;
   mEventType = type;
}

SipResourceList::SipResourceList(const char* bodyBytes, const char* type)
   : mRWMutex(OsRWMutex::Q_PRIORITY)
{
   remove(0);
   append(RESOURCE_LIST_CONTENT_TYPE);

   if(bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
   }
   
   ((SipResourceList*)this)->mBody = bodyBytes;   
   mEventType = type;
}


// Destructor
SipResourceList::~SipResourceList()
{
   // Clean up all the resource elements
   if (mResources.isEmpty())
   {
      mResources.destroyAll();
   }

   // Clean up all the event elements
   if (mEvents.isEmpty())
   {
      mEvents.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */

void SipResourceList::parseBody(const char* bodyBytes)
{
   if(bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::parseBody incoming package = %s\n", 
                    bodyBytes);
                    
      TiXmlDocument doc("ResourceList.xml");
      
      if (doc.Parse(bodyBytes))
      {
         TiXmlNode * rootNode = doc.FirstChild ("list");
        
         TiXmlElement* ucElement = 0;

      }
   }
}


// Assignment operator
SipResourceList&
SipResourceList::operator=(const SipResourceList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void SipResourceList::insertResource(Resource* resource)
{
   OsWriteLock lock(mRWMutex);
   if (mResources.insert(resource) != NULL)   
   {                 
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::insertResource Resource = %p", 
                    resource);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipResourceList::insertResource Resource = %p failed", 
                    resource);
   }
}


Resource* SipResourceList::removeResource(Resource* resource)
{
   OsWriteLock lock(mRWMutex);
   UtlContainable *foundValue;
   foundValue = mResources.remove(resource);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::removeResource Resource = %p", 
                 foundValue);                 

   return (Resource *) foundValue;
}


Resource* SipResourceList::getResource(UtlString& resourceUri)
{
   OsWriteLock lock(mRWMutex);
   UtlHashMapIterator resourceIterator(mResources);
   Resource* pResource;
   UtlString foundValue;
   while (pResource = (Resource *) resourceIterator())
   {
      pResource->getResourceUri(foundValue);
      
      if (foundValue.compareTo(resourceUri) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::getResource found Resource = %p for resourceUri %s", 
                       pResource, resourceUri.data());                 
            
         return pResource;
      }
   }     
          
   OsSysLog::add(FAC_SIP, PRI_WARNING, "SipResourceList::getDResource could not found the Resource for resourceUri = %s", 
                 resourceUri.data());                 
            
   return NULL;
}

UtlBoolean SipResourceList::isEmpty()
{
   OsWriteLock lock(mRWMutex);
   return (mResources.isEmpty());
}

void SipResourceList::insertEvent(UtlContainable* event)
{
   OsWriteLock lock(mRWMutex);
   if (mEvents.insert(event) != NULL)   
   {                 
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::insertEvent Event = %p", 
                    event);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipResourceList::insertEvent Event = %p failed", 
                    event);
   }
}


UtlContainable* SipResourceList::removeEvent(UtlContainable* event)
{
   OsWriteLock lock(mRWMutex);
   UtlContainable *foundValue;
   foundValue = mEvents.remove(event);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::removeEvent Event = %p", 
                 foundValue);                 

   return foundValue;
}


void SipResourceList::getEventType(UtlString& type) const
{
   type = mEventType;
}


void SipResourceList::getListUri(UtlString& uri) const
{
   uri = mListUri;
}


int SipResourceList::getLength() const
{
   int length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}

void SipResourceList::buildBody() const
{
   UtlString resourceList;
   char resourceListBuffer[MAX_CHAR_SIZE];
   char tempBuffer[MAX_CHAR_SIZE];

   // Construct the xml document of resource list
   resourceList = UtlString(XML_VERSION_1_0);

   //  Information Structure
   Url listUri(mListUri);
   sprintf(resourceListBuffer, "<list xmlns=\"%s\" uri=\"%s\" version=\"%d\" fullState=\"%s\">\n",
           RESOURCE_LIST_XMLNS, listUri.toString().data(), mVersion, mFullState.data());
   resourceList += UtlString(resourceListBuffer);
   
   // Resource elements
   OsWriteLock lock(((SipResourceList*)this)->mRWMutex);
   UtlHashMapIterator resourceIterator(mResources);
   Resource* pResource;
   while (pResource = (Resource *) resourceIterator())
   {
      UtlString uriStr;
      pResource->getResourceUri(uriStr);

      Url uri(uriStr);
      sprintf(resourceListBuffer, "<resource uri=\"%s\">\n", uri.toString().data());
      resourceList += UtlString(resourceListBuffer);

      // Name element
      UtlString name;
      pResource->getName(name);
      if (!name.isNull())
      {
         sprintf(resourceListBuffer, "<name>%s</name>\n", name.data());
      }
      resourceList += UtlString(resourceListBuffer);

      UtlString id, state;
      pResource->getInstance(id, state);
      sprintf(resourceListBuffer, "<instance id=\"%s\" state=\"%s\"/>\n",
              id.data(), state.data());
      resourceList += UtlString(resourceListBuffer);

      // End of resource element
      resourceList += UtlString("</resource>\n");
   }

   // End of list element
   resourceList += UtlString("</list>\n");
  
   ((SipResourceList*)this)->mBody = resourceList;
   ((SipResourceList*)this)->bodyLength = resourceList.length();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::getBytes Resource list content = \n%s", 
                 resourceList.data());                 
   ((SipResourceList*)this)->mVersion++;
}

void SipResourceList::getBytes(const char** bytes, int* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);
   ((SipResourceList*)this)->mBody = tempBody.data();

   *bytes = mBody.data();
}

void SipResourceList::getBytes(UtlString* bytes, int* length) const
{
   buildBody();
   
   *bytes = ((SipResourceList*)this)->mBody;
   *length = ((SipResourceList*)this)->bodyLength;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

