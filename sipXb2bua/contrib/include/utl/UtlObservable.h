//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

class UtlObservable;

/**
 * Generic interface for any object which is an observer
 * of an observable.
 */ 
class UtlObserver
{
public:
    /**
     * Notification method which the observer must implement.  The 
     * observable's (also, subject) resposibility is to notify 
     * the observers of state changes.  The observable will notify
     * the observers by invoking this onNotify method.
     *
     * @param subject The observable which invoked this method.
     * @param code An integer notification value.  For example, it
     *                  can be used to indicate the observable's state.
     * @param pUserData A reference to any object that the observable may
     *                  pass to the observer.
     */
    virtual void onNotify(UtlObservable* subject, int code, void *pUserData) = 0;
};

/**
 * Simple interface for any object which is observable,
 * meaning that observers can register to be notified
 * of the observable's state changes.
 */ 
class UtlObservable
{
public:
    /**
     * Registers a listener of this observable.
     * Derived classes of UtlObservable should store
     * its observers in a collection.
     *
     * @param observer The observer that wants to "listen to"
     *                 this observable's state changes.
     */ 
    virtual void registerObserver(UtlObserver* observer) = 0;

    /**
     * Removes a listener of this observable.
     * Derived classes of UtlObservable should remove
     * this observer from it's collection.
     *
     * @param observer The observer that no longer wants to "listen to"
     *                 this observable's state changes.
     */ 
    virtual void removeObserver(UtlObserver* observer) = 0;
protected:
    
    /**
     * The observable calls this to notify its
     * observers of a change.
     * Derived classes should iterate through their collection
     * of observers and call UtlObserver->onNotify .
     * 
     * @param code An integer notification value.  For example, it
     *                  can be used to indicate the observable's state.
     * @param pUserData A reference to any object that the observable may
     *                  pass to the observer.
     */ 
    virtual void notify(int code, void *pUserData) = 0;

};


