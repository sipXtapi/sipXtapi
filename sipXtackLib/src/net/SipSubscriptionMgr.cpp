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
#include <utl/UtlString.h>
#include <utl/UtlHashBagIterator.h>
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsDateTime.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipDialog.h>
#include <net/NetMd5Codec.h>


// Private class to contain callback for eventTypeKey
class SubscriptionServerState : public UtlString
{
public:
    SubscriptionServerState();

    virtual ~SubscriptionServerState();

    // Parent UtlString contains the dialog handle
    UtlString mResourceId;
    UtlString mEventTypeKey;
    UtlString mAcceptHeaderValue;
    long mExpirationDate; // epoch time
    SipMessage* mpLastSubscribeRequest;
    OsTimer* mpExpirationTimer;

private:
    //! DISALLOWED accidental copying
    SubscriptionServerState(const SubscriptionServerState& rSubscriptionServerState);
    SubscriptionServerState& operator=(const SubscriptionServerState& rhs);
};

class SubscriptionServerStateIndex : public UtlString
{
public:
    SubscriptionServerStateIndex();

    virtual ~SubscriptionServerStateIndex();

    // Parent UtlString contains the dialog handle
    SubscriptionServerState* mpState;


private:
    //! DISALLOWED accidental copying
    SubscriptionServerStateIndex(const SubscriptionServerStateIndex& rSubscriptionServerStateIndex);
    SubscriptionServerStateIndex& operator=(const SubscriptionServerStateIndex& rhs);
};


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SubscriptionServerState::SubscriptionServerState()
{
    mExpirationDate = -1;
    mpLastSubscribeRequest = NULL;
    mpExpirationTimer = NULL;
}
SubscriptionServerState::~SubscriptionServerState()
{
    if(mpLastSubscribeRequest)
    {
        delete mpLastSubscribeRequest;
        mpLastSubscribeRequest = NULL;
    }

    if(mpExpirationTimer)
    {
        // Timer should have been stopped and the the task upon
        // which the fired timer queues its message need to have
        // synchronized to make sure it does not get touched after
        // it is deleted here.
        delete mpExpirationTimer;
        mpExpirationTimer = NULL;
    }
}

SubscriptionServerStateIndex::SubscriptionServerStateIndex()
{
    mpState = NULL;
}

SubscriptionServerStateIndex::~SubscriptionServerStateIndex()
{
    // Do not delete mpState, it is freed else where
}

// Constructor
SipSubscriptionMgr::SipSubscriptionMgr(SipDialogMgr& dialogMgr)
: mSubscriptionMgrMutex(OsMutex::Q_FIFO)
{
    mpDialogMgr = &dialogMgr;
    mEstablishedDialogCount = 0;
    mMinExpiration = 32;
    mDefaultExpiration = 3600;
    mMaxExpiration = 86400;
}


// Copy constructor NOT IMPLEMENTED
SipSubscriptionMgr::SipSubscriptionMgr(const SipSubscriptionMgr& rSipSubscriptionMgr)
: mSubscriptionMgrMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipSubscriptionMgr::~SipSubscriptionMgr()
{
    // This crashed the SubscriptionMgrTest on Win32 but not
    // deleting mpDialogMgr causes a memory leak.
    // This is now causing a crash on Linux as well.  The whole thing
    // needs more investigation.
    delete mpDialogMgr ;

    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscriptionMgr& 
SipSubscriptionMgr::operator=(const SipSubscriptionMgr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipSubscriptionMgr::updateDialogInfo(const SipMessage& subscribeRequest,
                                                const UtlString& resourceId,
                                                const UtlString& eventTypeKey,
                                                OsMsgQ* subscriptionTimeoutQueue,
                                                UtlString& subscribeDialogHandle,
                                                UtlBoolean& isNew,
                                                UtlBoolean& isSubscriptionExpired,
                                                SipMessage& subscribeResponse)
{
    isNew = FALSE;
    UtlBoolean subscriptionSucceeded = FALSE;
    UtlString dialogHandle;
    subscribeRequest.getDialogHandle(dialogHandle);
    SubscriptionServerState* state = NULL;
    int expiration = -1;
    isSubscriptionExpired = TRUE;

    // If this is an early dialog we need to make it an established dialog.
    if(SipDialog::isEarlyDialog(dialogHandle))
    {
        UtlString establishedDialogHandle;
        if(mpDialogMgr->getEstablishedDialogHandleFor(dialogHandle, establishedDialogHandle))
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                "Incoming early SUBSCRIBE dialog: %s matches established dialog: %s",
                dialogHandle.data(), establishedDialogHandle.data());
        }

        // make up a To tag and set it
        UtlString toTagClearText;
        // Should probably add something like the local IP address and SIP port
        toTagClearText.append(dialogHandle);
        char numBuffer[20];
        lock();
        mEstablishedDialogCount++;
        sprintf(numBuffer, "%d", mEstablishedDialogCount);
        unlock();
        toTagClearText.append(numBuffer);
        UtlString toTag;
        NetMd5Codec::encode(toTagClearText, toTag);

        // Get and validate the expires period
        // This potentially should be delegated to the event handler specifics
        if(!subscribeRequest.getExpiresField(&expiration))
        {
            expiration = mDefaultExpiration;
        }

        else if(expiration > mMaxExpiration)
        {
            expiration = mMaxExpiration;
        }

        // Acceptable expiration, create a subscription and dialog
        if(expiration > mMinExpiration ||
           expiration == 0)
        {
            // Create a dialog and subscription state even if
            // the expiration is zero as we need the dialog info
            // to route the one time NOTIFY.  The immediately
            // expired dialog will be garbage collected.

            SipMessage* subscribeCopy = new SipMessage(subscribeRequest);
            subscribeCopy->setToFieldTag(toTag);

            // Re-get the dialog handle now that the To tag is set
            subscribeCopy->getDialogHandle(dialogHandle);

            // Create the dialog
            mpDialogMgr->createDialog(*subscribeCopy, FALSE, dialogHandle);
            isNew = TRUE;

            // Create a subscription state
            state = new SubscriptionServerState();
            *((UtlString*)state) = dialogHandle;
            state->mEventTypeKey = eventTypeKey;
            state->mpLastSubscribeRequest = subscribeCopy;
            state->mResourceId = resourceId;
            subscribeCopy->getAcceptField(state->mAcceptHeaderValue);

            long now = OsDateTime::getSecsSinceEpoch();
            state->mExpirationDate = now + expiration;

            // TODO: currently the SipSubsribeServer does not handle timeout
            // events to send notifications that the subscription has ended.
            // So we do not set a timer at the end of the subscription
            state->mpExpirationTimer = NULL;


            // Create the index by resourceId and eventTypeKey key
            SubscriptionServerStateIndex* stateKey = new SubscriptionServerStateIndex;
            *((UtlString*)stateKey) = resourceId;
            stateKey->append(eventTypeKey);
            stateKey->mpState = state;

            // Set the contact to the same request URI that came in
            UtlString contact;
            subscribeRequest.getRequestUri(&contact);

            // Add the angle brackets for contact
            Url url(contact);
            url.includeAngleBrackets();
            contact = url.toString();

            subscribeResponse.setResponseData(subscribeCopy, 
                                            SIP_ACCEPTED_CODE,
                                            SIP_ACCEPTED_TEXT, 
                                            contact);
            subscribeResponse.setExpiresField(expiration);
            subscribeCopy->getDialogHandle(subscribeDialogHandle);

            lock();
            mSubscriptionStatesByDialogHandle.insert(state);
            mSubscriptionStateResourceIndex.insert(stateKey);
            // Not safe to touch these after we unlock
            stateKey = NULL;
            state = NULL;
            subscribeCopy = NULL;
            unlock();

            subscriptionSucceeded = TRUE;

            // One time subscribe
            if(expiration == 0)
            {
                isSubscriptionExpired = TRUE;
            }
            else
            {
                isSubscriptionExpired = FALSE;
            }
        }


        // Expiration too small
        else
        {
            // Set expiration too small error
            subscribeResponse.setResponseData(&subscribeRequest, 
                                                SIP_TOO_BRIEF_CODE,
                                                SIP_SUB_TOO_BRIEF_TEXT);
            subscribeResponse.setMinExpiresField(mMinExpiration);
            isSubscriptionExpired = TRUE;

        }

    }

    // The dialog for this message should already exist
    else
    {
        // Get and validate the expires period
        // This potentially should be delegated to the event handler specifics
        if(!subscribeRequest.getExpiresField(&expiration))
        {
            expiration = mDefaultExpiration;
        }

        else if(expiration > mMaxExpiration)
        {
            expiration = mMaxExpiration;
        }

        // Acceptable expiration, create a subscription and dialog
        if(expiration > mMinExpiration ||
           expiration == 0)
        {
            // Update the dialog state
            mpDialogMgr->updateDialog(subscribeRequest, dialogHandle);

            // Get the subscription state and update that
            // TODO:  This assumes that no one reuses the same dialog
            // to subscribe to more than one event type.  mSubscriptionStatesByDialogHandle
            // will need to be changed to a HashBag and we will need to
            // search through to find a matching event type
            lock();
            state = (SubscriptionServerState*)
                mSubscriptionStatesByDialogHandle.find(&dialogHandle);
            if(state)
            {
                // 20/12/2005 - Joanne Brunet
                long now = OsDateTime::getSecsSinceEpoch();
                state->mExpirationDate = now + expiration;
                if(state->mpLastSubscribeRequest)
                {
                    delete state->mpLastSubscribeRequest;
                }
                state->mpLastSubscribeRequest = new SipMessage(subscribeRequest);
                subscribeRequest.getAcceptField(state->mAcceptHeaderValue);

                // Set the contact to the same request URI that came in
                UtlString contact;
                subscribeRequest.getRequestUri(&contact);

                // Add the angle brackets for contact
                Url url(contact);
                url.includeAngleBrackets();
                contact = url.toString();

                subscribeResponse.setResponseData(&subscribeRequest, 
                                                SIP_ACCEPTED_CODE,
                                                SIP_ACCEPTED_TEXT, 
                                                contact);
                subscribeResponse.setExpiresField(expiration);
                subscriptionSucceeded = TRUE;
                isSubscriptionExpired = FALSE;
                subscribeDialogHandle = dialogHandle;
            }

            // No state, basically assume this is a new subscription
            else
            {
                SipMessage* subscribeCopy = new SipMessage(subscribeRequest);

                // Create the dialog
                mpDialogMgr->createDialog(*subscribeCopy, FALSE, dialogHandle);
                isNew = TRUE;

                // Create a subscription state
                state = new SubscriptionServerState();
                *((UtlString*)state) = dialogHandle;
                state->mEventTypeKey = eventTypeKey;
                state->mpLastSubscribeRequest = subscribeCopy;
                state->mResourceId = resourceId;
                subscribeCopy->getAcceptField(state->mAcceptHeaderValue);

                long now = OsDateTime::getSecsSinceEpoch();
                state->mExpirationDate = now + expiration;
                // TODO: currently the SipSubsribeServer does not handle timeout
                // events to send notifications that the subscription has ended.
                // So we do not set a timer at the end of the subscription
                state->mpExpirationTimer = NULL;

                // Create the index by resourceId and eventTypeKey key
                SubscriptionServerStateIndex* stateKey = new SubscriptionServerStateIndex;
                *((UtlString*)stateKey) = resourceId;
                stateKey->append(eventTypeKey);
                stateKey->mpState = state;
                mSubscriptionStatesByDialogHandle.insert(state);
                mSubscriptionStateResourceIndex.insert(stateKey);
                // Not safe to touch these after we unlock
                stateKey = NULL;
                state = NULL;
                subscribeCopy = NULL;

                // Set the contact to the same request URI that came in
                UtlString contact;
                subscribeRequest.getRequestUri(&contact);

                // Add the angle brackets for contact
                Url url(contact);
                url.includeAngleBrackets();
                contact = url.toString();

                subscribeResponse.setResponseData(&subscribeRequest, 
                                                SIP_ACCEPTED_CODE,
                                                SIP_ACCEPTED_TEXT, 
                                                contact);
                subscribeResponse.setExpiresField(expiration);
                subscriptionSucceeded = TRUE;
                // Unsubscribe
                if(expiration == 0)
                {
                    isSubscriptionExpired = TRUE;
                }
                else
                {
                    isSubscriptionExpired = FALSE;
                }
                subscribeDialogHandle = dialogHandle;
            }
            unlock();
        }

        // Expiration too small
        else
        {
            // Set expiration too small error
            subscribeResponse.setResponseData(&subscribeRequest, 
                                                SIP_TOO_BRIEF_CODE,
                                                SIP_SUB_TOO_BRIEF_TEXT);
            subscribeResponse.setMinExpiresField(mMinExpiration);
            isSubscriptionExpired = isExpired(dialogHandle);
        }
        
    }

    return(subscriptionSucceeded);
}

UtlBoolean SipSubscriptionMgr::getNotifyDialogInfo(const UtlString& subscribeDialogHandle,
                                                   SipMessage& notifyRequest)
{
    UtlBoolean notifyInfoSet = FALSE;
    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&subscribeDialogHandle);

    if(state)
    {
        notifyInfoSet = mpDialogMgr->setNextLocalTransactionInfo(notifyRequest, 
                                                             SIP_NOTIFY_METHOD,
                                                             subscribeDialogHandle);

        if(state->mpLastSubscribeRequest)
        {
            UtlString eventHeader;
            state->mpLastSubscribeRequest->getEventField(eventHeader);
            notifyRequest.setEventField(eventHeader);
        }
    }
    unlock();

    return(notifyInfoSet);
}

UtlBoolean SipSubscriptionMgr::createNotifiesDialogInfo(const char* resourceId,
                                                        const char* eventTypeKey,
                                                        int& numNotifiesCreated,
                                                        UtlString**& acceptHeaderValuesArray,
                                                        SipMessage**& notifyArray)
{
    UtlString contentKey(resourceId);
    contentKey.append(eventTypeKey);

    lock();
    UtlHashBagIterator iterator(mSubscriptionStateResourceIndex, &contentKey);
    int count = 0;
    int index = 0;
    acceptHeaderValuesArray = NULL;
    notifyArray = NULL;

    while(iterator())
    {
        count++;
    }

    if(count > 0)
    {
        SubscriptionServerStateIndex* contentTypeIndex = NULL;
        acceptHeaderValuesArray = new UtlString*[count];
        notifyArray = new SipMessage*[count];
        iterator.reset();
        long now = OsDateTime::getSecsSinceEpoch();

        while((contentTypeIndex = (SubscriptionServerStateIndex*)iterator()))
        {
            // Should not happen, the container is supposed to be locked
            if(index >= count)
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipSubscriptionMgr::createNotifiesDialogInfo iterator elements count changed from: %d to %d while locked",
                    count, index);
            }
            // Should not happen, the index should be created and
            // deleted with the state
            else if(contentTypeIndex->mpState == NULL)
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipSubscriptionMgr::createNotifiesDialogInfo SubscriptionServerStateIndex with NULL mpState");
            }

            // If not expired yet
            else if(contentTypeIndex->mpState->mExpirationDate >= now)
            {
                acceptHeaderValuesArray[index] = 
                    new UtlString(contentTypeIndex->mpState->mAcceptHeaderValue);
                notifyArray[index] = new SipMessage;
                if (true == 
                    mpDialogMgr->setNextLocalTransactionInfo(*(notifyArray[index]),
                                                         SIP_NOTIFY_METHOD, 
                                                         *(contentTypeIndex->mpState)))
                {                                                         

                    UtlString eventHeader;
                    if(contentTypeIndex->mpState->mpLastSubscribeRequest)
                    {
                        contentTypeIndex->mpState->mpLastSubscribeRequest->getEventField(eventHeader);
                    }
                    (notifyArray[index])->setEventField(eventHeader);

                    index++;
                }
            }
        }
    }
    unlock();

    numNotifiesCreated = index;

    return(index > 0);
}

void SipSubscriptionMgr::freeNotifies(int numNotifies,
                                      UtlString** acceptHeaderValues,
                                      SipMessage** notifiesArray)
{
    if(notifiesArray && acceptHeaderValues)
    {
        if(numNotifies > 0)
        {
            for(int index = 0; index < numNotifies; index++)
            {
                if(acceptHeaderValues[index])
                {
                    delete acceptHeaderValues[index];
                }
                if(notifiesArray[index])
                {
                    delete notifiesArray[index];
                }
            }
            delete[] acceptHeaderValues;
            delete[] notifiesArray;
        }

        // Should not have array with a count of zero
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscriptionMgr::freeNotifies zero length arrays not NULL");
        }
    }
}

UtlBoolean SipSubscriptionMgr::endSubscription(const UtlString& dialogHandle)
{
    UtlBoolean subscriptionFound = FALSE;

    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&dialogHandle);
    if(state)
    {
        SubscriptionServerStateIndex* stateIndex = NULL;
        UtlString contentKey(state->mResourceId);
        contentKey.append(state->mEventTypeKey);
        UtlHashBagIterator iterator(mSubscriptionStateResourceIndex, &contentKey);
        while((stateIndex = (SubscriptionServerStateIndex*) iterator()))
        {
            if(stateIndex->mpState == state)
            {
                mSubscriptionStatesByDialogHandle.removeReference(state);
                mSubscriptionStateResourceIndex.removeReference(stateIndex);

                delete state;
                delete stateIndex;
                subscriptionFound = TRUE;

                break;
            }
        }

        // Could not find the state index that cooresponded to the state
        // SHould not happen, there should always be one of each
        if(!subscriptionFound)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscriptionMgr::endSubscription could not find SubscriptionServerStateIndex for state with dialog: %s",
                dialogHandle.data());
        }
    }

    unlock();

    // Remove the dialog
    mpDialogMgr->deleteDialog(dialogHandle);

    return(subscriptionFound);
}

void SipSubscriptionMgr::removeOldSubscriptions(long oldEpochTimeSeconds)
{
    lock();
    UtlHashBagIterator iterator(mSubscriptionStateResourceIndex);
    SubscriptionServerStateIndex* stateIndex = NULL;
    while((stateIndex = (SubscriptionServerStateIndex*) iterator()))
    {
        if(stateIndex->mpState)
        {
            if(stateIndex->mpState->mExpirationDate < oldEpochTimeSeconds)
            {
                mpDialogMgr->deleteDialog(*(stateIndex->mpState));
                mSubscriptionStatesByDialogHandle.removeReference(stateIndex->mpState);
                delete stateIndex->mpState;
                stateIndex->mpState = NULL;
                mSubscriptionStateResourceIndex.removeReference(stateIndex);
                delete stateIndex;
            }
        }

        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscriptionMgr::removeOldSubscriptions SubscriptionServerStateIndex with NULL mpState, deleting");
            mSubscriptionStateResourceIndex.removeReference(stateIndex);
            delete stateIndex;
        }
    }

    unlock();
}

/* ============================ ACCESSORS ================================= */

SipDialogMgr* SipSubscriptionMgr::getDialogMgr()
{
    return(mpDialogMgr);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipSubscriptionMgr::dialogExists(UtlString& dialogHandle)
{
    UtlBoolean subscriptionFound = FALSE;

    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&dialogHandle);
    if(state)
    {
        subscriptionFound = TRUE;
    }
    unlock();

    return(subscriptionFound);
}

UtlBoolean SipSubscriptionMgr::isExpired(UtlString& dialogHandle)
{
    UtlBoolean subscriptionExpired = TRUE;

    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&dialogHandle);
    if(state)
    {
        long now = OsDateTime::getSecsSinceEpoch();

        if(now <= state->mExpirationDate)
        {
            subscriptionExpired = FALSE;
        }
    }
    unlock();

    return(subscriptionExpired);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void SipSubscriptionMgr::lock()
{
    mSubscriptionMgrMutex.acquire();
}

void SipSubscriptionMgr::unlock()
{
    mSubscriptionMgrMutex.release();
}

/* ============================ FUNCTIONS ================================= */

