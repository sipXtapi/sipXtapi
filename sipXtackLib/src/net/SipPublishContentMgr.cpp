//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

//#define TEST_PRINT 

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipPublishContentMgr.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <net/HttpBody.h>
#include <os/OsSysLog.h>


// Private class to contain callback for eventTypeKey
class PublishCallbackContainer : public UtlString
{
public:
    PublishCallbackContainer();

    virtual ~PublishCallbackContainer();

    // Parent UtlString contains the SIP event Type token (not the eventTypeKey)
    void* mpApplicationData;
    SipPublishContentMgr::SipPublisherContentChangeCallback mpCallback;

private:
    //! DISALLOWED accidental copying
    PublishCallbackContainer(const PublishCallbackContainer& rPublishCallbackContainer);
    PublishCallbackContainer& operator=(const PublishCallbackContainer& rhs);
};

// Private class to contain event content for eventTypeKey
class PublishContentContainer : public UtlString
{
public:
    PublishContentContainer();

    virtual ~PublishContentContainer();

    // parent UtlString contains the resourceId and eventTypeKey
    UtlSList mEventContent;


private:
    //! DISALLOWED accendental copying
    PublishContentContainer(const PublishContentContainer& rPublishContentContainer);
    PublishContentContainer& operator=(const PublishContentContainer& rhs);

};

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
PublishCallbackContainer::PublishCallbackContainer() :
   mpApplicationData(NULL),
   mpCallback(NULL)
{
}
PublishCallbackContainer::~PublishCallbackContainer()
{
}

PublishContentContainer::PublishContentContainer()
{
}
PublishContentContainer::~PublishContentContainer()
{
}

// Constructor
SipPublishContentMgr::SipPublishContentMgr()
: mPublishMgrMutex(OsMutex::Q_FIFO)
{
}


// Copy constructor NOT IMPLEMENTED
SipPublishContentMgr::SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr)
: mPublishMgrMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipPublishContentMgr::~SipPublishContentMgr()
{
   // Delete the stored information.
   mContentEntries.destroyAll();
   mDefaultContentEntries.destroyAll();
   mDefaultContentConstructors.destroyAll();
   mEventContentCallbacks.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipPublishContentMgr& 
SipPublishContentMgr::operator=(const SipPublishContentMgr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipPublishContentMgr::publish(const char* resourceId,
                                         const char* eventTypeKey,
                                         const char* eventType,
                                         int numContentTypes,
                                         HttpBody* eventContent[],
                                   UtlBoolean noNotify)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publish resourceId '%s', eventTypeKey '%s', eventType '%s', numContentTypes %d",
                  resourceId, eventTypeKey, eventType, numContentTypes);
#ifdef TEST_PRINT
    osPrintf("SipPublishContentMgr::publish(%s, %s, %s, %d, [%p], ...)\n",
        resourceId, eventTypeKey, eventType, numContentTypes, eventContent[0]);
#endif

    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
    }

    if(eventTypeKey)
    {
        key.append(eventTypeKey);
    }

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> ((resourceIdProvided ?
                                                 mContentEntries :
                                                 mDefaultContentEntries).find(&key));

    // If not found, create a container.
    if(container == NULL)
    {
        container = new PublishContentContainer();
        *((UtlString*) container) = key;
	// Save the container in the appropriate hash.
        (resourceIdProvided ?
	 mContentEntries :
	 mDefaultContentEntries).insert(container);
    }

    // The content for this event type already existed
    else
    {
        // Remove the old content
	container->mEventContent.destroyAll();
    }

    // Add the new content
    for(int index = 0; index < numContentTypes; index++)
        {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipPublishContentMgr::publish eventContent[%d] = %p",
                      index, eventContent[index]);
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipPublishContentMgr::publish eventContent[%d]->getBytes() = %p",
                      index, eventContent[index]->getBytes());
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipPublishContentMgr::publish eventContent[%d] = '%s'",
                      index, eventContent[index]->getBytes());
        container->mEventContent.append(eventContent[index]);
        }

    // Don't call the observers if noNotify is set.
    if (!noNotify)
    {
       // Call the observer for the content change, if any.
       UtlString eventTypeString(eventType);
       PublishCallbackContainer* callbackContainer =
          dynamic_cast <PublishCallbackContainer*>
          (mEventContentCallbacks.find(&eventTypeString));
       if(callbackContainer && callbackContainer->mpCallback)
        {
          (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                          resourceId,
                                          eventTypeKey,
                                          eventTypeString,
                                          !resourceIdProvided);
        }
    }

    unlock();
    }

void SipPublishContentMgr::publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          int numContentTypes,
                                          HttpBody* eventContent[])
    {
    publish(NULL,
            eventTypeKey,
            eventType,
            numContentTypes,
            eventContent);
}

void SipPublishContentMgr::publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          SipPublishContentMgrDefaultConstructor*
                                          defaultConstructor)
        {
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publishDefault eventTypeKey '%s', eventType '%s', defaultConstructor %p",
                  eventTypeKey, eventType, defaultConstructor);
    // Construct the key to look up.
    UtlString key;

    if(eventTypeKey)
            {
       key.append(eventTypeKey);
            }

    lock();

    // Add the default constructor.
    if (defaultConstructor)
                {
       // Remove any old value first.
       mDefaultContentConstructors.destroy(&key);
       UtlString* key_heap = new UtlString(key);
       mDefaultContentConstructors.insertKeyAndValue(key_heap,
                                                     defaultConstructor);
                }

    // Call the observer for the content change, if any.
    UtlString eventTypeString(eventType);
    PublishCallbackContainer* callbackContainer =
       dynamic_cast <PublishCallbackContainer*>
       (mEventContentCallbacks.find(&eventTypeString));
    if(callbackContainer && callbackContainer->mpCallback)
                {
        (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                        NULL,
                                        eventTypeKey,
                                        eventTypeString,
                                        TRUE);
                }

    unlock();
            }

void SipPublishContentMgr::unpublish(const char* resourceId,
                                     const char* eventTypeKey,
                                     const char* eventType)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::unpublish resourceId '%s', eventTypeKey '%s', eventType '%s'",
                  resourceId, eventTypeKey, eventType);
    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
        }

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
            dynamic_cast <PublishContentContainer*> ((resourceIdProvided ?
                                                      mContentEntries :
                                                      mDefaultContentEntries).find(&key));

    // If a container was found, delete it and its contents.
    if (container)
        {
	container->mEventContent.destroyAll();
        (resourceIdProvided ?
         mContentEntries :
         mDefaultContentEntries).destroy(container);
        }

    // Remove any default constructor.
    if (!resourceIdProvided)
    {
       mDefaultContentConstructors.destroy(&key);
    }

    // Call the observer for the content change, if any.
        UtlString eventTypeString(eventType);
    PublishCallbackContainer* callbackContainer =
       dynamic_cast <PublishCallbackContainer*>
       (mEventContentCallbacks.find(&eventTypeString));
        if(callbackContainer && callbackContainer->mpCallback)
        {
            (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                          resourceId,
                                          eventTypeKey,
                                          eventTypeString,
                                          !resourceIdProvided);
        }

    unlock();
}

void SipPublishContentMgr::unpublishDefault(const char* eventTypeKey,
                                            const char* eventType)
{
    unpublish(NULL,
                   eventTypeKey,
              eventType);
}

UtlBoolean SipPublishContentMgr::setContentChangeObserver(const char* eventType,
                                                          void* applicationData,
                                                          SipPublisherContentChangeCallback callbackFunction)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::setContentChangeObserver eventType '%s', callbackFunction %p",
                  eventType, callbackFunction);
    UtlBoolean callbackSet = FALSE;
    UtlString eventTypeString(eventType);

    lock();

    // eventTypeKey and eventType must be defined
    if(eventType == NULL ||
       *eventType == 0)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, event type not set.");

    }

    // There should not be a callback set, need to unset first
    else if((mEventContentCallbacks.find(&eventTypeString)))
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, callback already exists for event: %s",
            eventType ? eventType : "<null>");
    }

    else
    {
        callbackSet = TRUE;
        PublishCallbackContainer* callbackEntry = new PublishCallbackContainer();
        *((UtlString*)callbackEntry) = eventType;
        callbackEntry->mpApplicationData = applicationData;
        callbackEntry->mpCallback = callbackFunction;
        mEventContentCallbacks.insert(callbackEntry);
    }

    unlock();

    return(callbackSet);
}



UtlBoolean SipPublishContentMgr::removeContentChangeObserver(const char* eventType,
                                                             void*& applicationData,
                                SipPublisherContentChangeCallback& callbackFunction)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::removeContentChangeObserver eventType '%s', callbackFunction %p",
                  eventType, callbackFunction);
    UtlBoolean callbackRemoved = FALSE;
    UtlString eventTypeString(eventType);
    PublishCallbackContainer* callbackEntry = NULL;

    lock();

    // eventType must be defined
    if(eventType == NULL ||
       *eventType == 0)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, event type not set.");

    }

    // There should not be a callback set, need to unset first
    else if((callbackEntry = (PublishCallbackContainer*) 
        mEventContentCallbacks.remove(&eventTypeString)))
    {
        callbackRemoved = TRUE;
        callbackFunction = callbackEntry->mpCallback;
        applicationData = callbackEntry->mpApplicationData;
        delete callbackEntry;
        callbackEntry = NULL;
    }

    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, no callback exists for event: %s",
            eventType ? eventType : "<null>");
    }

    unlock();

    return(callbackRemoved);
}


/* ============================ ACCESSORS ================================= */

UtlBoolean SipPublishContentMgr::getContent(const char* resourceId,
                                          const char* eventTypeKey,
                                          const char* eventType,
                                          const char* acceptHeaderValue,
                                          HttpBody*& content,
                                          UtlBoolean& isDefaultContent)
{
#ifdef TEST_PRINT
    osPrintf("SipPublishContentMgr::getContent(%s, %s, %s, ...)\n",
        resourceId, eventTypeKey, acceptHeaderValue);
#endif

    UtlBoolean foundContent = FALSE;
    UtlString key(resourceId);

    key.append(eventTypeKey);
    PublishContentContainer* container = NULL;
    UtlHashMap contentTypes;
    isDefaultContent = FALSE;

    UtlBoolean acceptedTypesGiven = buildContentTypesContainer(acceptHeaderValue, contentTypes);

    lock();
    // See if resource specific content exists
    container = 
        dynamic_cast <PublishContentContainer*> (mContentEntries.find(&key));

    // There is no resource specific content.  Check if the default
    // constructor exists.
    if(container == NULL)
    {
       // Construct the key for the default data.
       UtlString default_key(eventTypeKey);

       // Look up the constructor.

       SipPublishContentMgrDefaultConstructor* constructor =
          dynamic_cast <SipPublishContentMgrDefaultConstructor*>
          (mDefaultContentConstructors.findValue(&default_key));
       // If it exists, call it to publish content for this resource/event.
       if (constructor)
       {
          constructor->generateDefaultContent(this, resourceId,
                                              eventTypeKey, eventType);
       }

       // See if resource specific content exists now.
        container = 
          dynamic_cast <PublishContentContainer*> (mContentEntries.find(&key));

       // If content was found, still mark it as default content.
        if(container)
        {
            isDefaultContent = TRUE;
        }
       // If still no content was found, check if the default exists.
       else
       {
           container = 
              dynamic_cast <PublishContentContainer*>
              (mDefaultContentEntries.find(&default_key));
           if(container)
           {
               isDefaultContent = TRUE;
           }
       }
    }

    if(container)
    {
        HttpBody* bodyPtr = NULL;
        UtlSListIterator contentIterator(container->mEventContent);
        while((bodyPtr = (HttpBody*)contentIterator()))
        {
            // No MIME types specified, take the first one
            if(!acceptedTypesGiven)
            {
                content = HttpBody::copyBody(*bodyPtr);
                foundContent = TRUE;
                break;
            }

            // Find the first match.  The container has the bodies
            // in the server's preferred order.
            if(contentTypes.find(bodyPtr))
            {
                content = HttpBody::copyBody(*bodyPtr);
                foundContent = TRUE;
                break;
            }
        }
    }
    else
    {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                     "SipPublishContentMgr::getContent no container is found for acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s'",
                     acceptHeaderValue ? acceptHeaderValue : "[none]",
                     resourceId ? resourceId : "[none]",
                     eventTypeKey, eventType);
    }

    unlock();

    contentTypes.destroyAll();
    return(foundContent);
}


void SipPublishContentMgr::getStats(int& numDefaultContent,
                                    int& numDefaultConstructor,
                                    int& numResourceSpecificContent,
                                    int& numCallbacksRegistered)
{
    lock();
    numDefaultContent = mDefaultContentEntries.entries();
    numDefaultConstructor = mDefaultContentConstructors.entries();
    numResourceSpecificContent = mContentEntries.entries();
    numCallbacksRegistered = mEventContentCallbacks.entries();
    unlock();
}

UtlBoolean SipPublishContentMgr::getPublished(const char* resourceId,
                                              const char* eventTypeKey,
                                              int maxContentTypes,
                                              int& numContentTypes,
                                              HttpBody* eventContent[],
                                              SipPublishContentMgrDefaultConstructor**
                                              pDefaultConstructor)
{
    UtlBoolean contentReturned = FALSE;

    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
    }

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> ((resourceIdProvided ?
                                                 mContentEntries :
                                                 mDefaultContentEntries).find(&key));

    // If not found, return zero versions.
    if (container == NULL)
    {
       contentReturned = TRUE;
       numContentTypes = 0;
    }

    // The content for this event type exists.
    else
    {
        int num = container->mEventContent.entries();

        if(num <= maxContentTypes)
        {
           contentReturned = TRUE;
           numContentTypes = num;
           // Copy the contents into the array.
           for (int index = 0; index < num; index++)
           {
              eventContent[index] =
                 new HttpBody(*dynamic_cast <HttpBody*>
                               (container->mEventContent.at(index)));
           }
        }
    }

    // Return the default constructor, if any.
    if (pDefaultConstructor)
    {
       UtlContainable* defaultConstructor =
          mDefaultContentConstructors.findValue(&key);
       *pDefaultConstructor =
          // Is there a default constructor?
          defaultConstructor ?
          // If so, make a copy of the constructor and return pointer to it.
          (dynamic_cast <SipPublishContentMgrDefaultConstructor*>
           (defaultConstructor))->copy() :
          // Otherwise, return NULL.
          NULL;
    }

    unlock();

    return contentReturned;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean
SipPublishContentMgr::buildContentTypesContainer(const char* acceptHeaderValue, 
                                                 UtlHashMap& contentTypes)
{
    // TODO: need to parse the comma delimited mime types

    UtlBoolean containsMimetypes = FALSE;
    // Parse the accept header value and add a string for each mime type
    // For each mime type
    //while(mimeType = parsed from acceptHeaderValue)
    //{

    // Add a string to the list to contain the MIME type
    //    contentTypes.insert(new UtlString(mimeType));
    //    containsMimetypes = TRUE;
    //}
    
    return(containsMimetypes);
}

void SipPublishContentMgr::lock()
{
    mPublishMgrMutex.acquire();
}

void SipPublishContentMgr::unlock()
{
    mPublishMgrMutex.release();
}

/* ============================ FUNCTIONS ================================= */
