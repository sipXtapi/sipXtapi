//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

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
    UtlString mResourceId;
    UtlString mEventTypeKey;
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
PublishCallbackContainer::PublishCallbackContainer()
{
    mpApplicationData = NULL;
    mpCallback = NULL;
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
    // Iterate through and delete all the dialogs
    // TODO:
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

UtlBoolean SipPublishContentMgr::publishDefault(const char* eventTypeKey,
                                                const char* eventType,
                                                int numContentTypes,
                                                HttpBody* eventContent[],
                                                int maxOldContentTypes,
                                                int& numOldContentTypes,
                                                HttpBody* oldEventContent[])
{
    return(publish(NULL,
                   eventTypeKey,
                   eventType,
                   numContentTypes,
                   eventContent,
                   maxOldContentTypes,
                   numOldContentTypes,
                   oldEventContent));
}

UtlBoolean SipPublishContentMgr::publish(const char* resourceId,
                                         const char* eventTypeKey,
                                         const char* eventType,
                                         int numContentTypes,
                                         HttpBody* eventContent[],
                                         int maxOldContentTypes,
                                         int& numOldContentTypes,
                                         HttpBody* oldEventContent[])
{
    UtlBoolean contentAdded = FALSE;
    UtlBoolean resourceIdProvided = FALSE;
    UtlString key;

    if(resourceId && *resourceId)
    {
        resourceIdProvided = TRUE;
        key = resourceId;
    }

    if(eventTypeKey)
    {
        key.append(eventTypeKey);
    }

    int numOld = 0;
    lock();
    PublishContentContainer* container = NULL;
    if(resourceIdProvided)
    {
        // See if resource specific content already exists
        container = 
            (PublishContentContainer*) mContentEntries.find(&key);
    }
    else
    {
        // See if default content already exists
        container = 
            (PublishContentContainer*) mDefaultContentEntries.find(&key);
    }

    if(container == NULL && numContentTypes > 0)
    {
        container = new PublishContentContainer();
        *((UtlString*)container) = key;
        container->mResourceId = "";
        container->mEventTypeKey = eventTypeKey;
        numOldContentTypes = 0;
        if(resourceIdProvided)
        {
            // Add resource specific content
            mContentEntries.insert(container);
        }
        else
        {
            // Add default content
            mDefaultContentEntries.insert(container);
        }
    }

    // No existing content and no new content
    else if(container == NULL && numContentTypes <= 0)
    {
    }

    // The content default for this event type already existed
    else
    {
        numOld = container->mEventContent.entries();

        if(numOld <= maxOldContentTypes)
        {
            // Remove the old content
            numOldContentTypes = numOld;
            for(int index = 0; index < numOld; index++)
            {
                oldEventContent[index] = (HttpBody*)
                    container->mEventContent.first();
                container->mEventContent.remove(oldEventContent[index]);
            }

            if(numContentTypes <= 0)
            {
                // Remove and delete the container
                if(resourceIdProvided)
                {
                    // Remove resource specific content
                    mContentEntries.remove(container);
                }
                else
                {
                    // Remove default content
                    mDefaultContentEntries.remove(container);
                }
                delete container;
                container = NULL;
            }
        }
    }


    // If there is old content and we did not get a big enough array
    // to put the old content in, don't do anything
    if(numOld <= maxOldContentTypes)
    {
        // Add the new content
        for(int index = 0; index < numContentTypes; index++)
        {
            container->mEventContent.append(eventContent[index]);
        }
        contentAdded = TRUE;


        // See if there is a observer for the content change
        UtlString eventTypeString(eventType);
        PublishCallbackContainer* callbackContainer = (PublishCallbackContainer*)
            mEventContentCallbacks.find(&eventTypeString);
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

    return(contentAdded);
}




UtlBoolean SipPublishContentMgr::unpublish(const char* resourceId,
                                           const char* eventTypeKey,
                                           const char* eventType,
                                           int maxOldContentTypes,
                                           int& numOldContentTypes,
                                           HttpBody* oldEventContent[])
{
    return(publish(resourceId,
                   eventTypeKey,
                   eventType,
                   0,
                   NULL,
                   maxOldContentTypes,
                   numOldContentTypes,
                   oldEventContent));
}

UtlBoolean SipPublishContentMgr::unpublishDefault(const char* eventTypeKey,
                                                  const char* eventType,
                                                   int maxOldContentTypes,
                                                   int& numOldContentTypes,
                                                   HttpBody* oldEventContent[])
{
    return(publish(NULL,
                   eventTypeKey,
                   eventType,
                   0,
                   NULL,
                   maxOldContentTypes,
                   numOldContentTypes,
                   oldEventContent));
}

UtlBoolean SipPublishContentMgr::setContentChangeObserver(const char* eventType,
                                                          void* applicationData,
                                                          SipPublisherContentChangeCallback callbackFunction)
{
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
                                          const char* acceptHeaderValue,
                                          HttpBody*& content,
                                          UtlBoolean& isDefaultContent)
{
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
        (PublishContentContainer*) mContentEntries.find(&key);

    // No resource specific content check if the default exists
    if(container == NULL)
    {
        key = eventTypeKey;
        container = 
            (PublishContentContainer*) mDefaultContentEntries.find(&key);

        if(container)
        {
            isDefaultContent = TRUE;
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
                content = new HttpBody(*bodyPtr);
                foundContent = TRUE;
                break;
            }

            // Find the first match.  The container has the bodies
            // in the servers preferred order.
            if(contentTypes.find(bodyPtr))
            {
                content = new HttpBody(*bodyPtr);
                foundContent = TRUE;
                break;
            }
        }
    }
    else
    {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                  "SipPublishContentMgr::getContent no container is found\n");
                  
    }

    unlock();

    contentTypes.destroyAll();
    return(foundContent);
}


void SipPublishContentMgr::getStats(int& numDefaultContent,
                                    int& numResourceSpecificContent,
                                    int& numCallbacksRegistered)
{
    lock();
    numDefaultContent = mDefaultContentEntries.entries();
    numResourceSpecificContent = mContentEntries.entries();
    numCallbacksRegistered = mEventContentCallbacks.entries();
    unlock();
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
