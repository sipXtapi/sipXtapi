//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlObservableImpl_h_
#define _UtlObservableImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsMutex.h"
#include "utl/UtlObservable.h"
#include "utl/UtlSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class UtlObservableImpl : public UtlObservable
{
public:
    /**
     * Constructor.
     */
    UtlObservableImpl();
    
    /**
     * Implementation of UtlObservable::registerObserver.
     * 
     * @param observer The observer that wants to "listen to"
     *                 this observable's state changes.
     */ 
    virtual void registerObserver(UtlObserver* observer) = 0;

    /**
     * Implementation of UtlObservable::removeObserver.
     * 
     * @param observer The observer that no longer wants to "listen to"
     *                 this observable's state changes.
     */ 
    virtual void removeObserver(UtlObserver* observer) = 0;
protected:
    
    /**
     * Implementation of UtlObservable::notify.
     * 
     * @param code An integer notification value.  For example, it
     *                  can be used to indicate the observable's state.
     * @param pUserData A reference to any object that the observable may
     *                  pass to the observer.
     */ 
    virtual void notify(int code, void *pUserData);    
private:
    /**
     * Collection of observers.
     */
    UtlSList mObservers;
    
    /**
     *  For synchronization locking
     */
    OsMutex mMutex;
};

#endif
