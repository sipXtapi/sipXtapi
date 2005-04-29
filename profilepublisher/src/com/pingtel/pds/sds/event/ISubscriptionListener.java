/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

 
package com.pingtel.pds.sds.event ;

/**
 * Subscription/Enrollment listener interface for the SipConfigServerAgent.  
 * These methods are invoked whenever a client subscribes to unsubscribes 
 * from the agent.
 *
 * @see SubscriptionEvent
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public interface ISubscriptionListener
{
    /**
     * Invoked whenever a subscription is added/requested.  This subscription 
     * may be a new subscription or an already active subscription being 
     * renewed.
     */
    public void subscriptionAdded(SubscriptionEvent event) ;
    
    
    /**
     * Invoked whenever a subscription is removed.
     */    
    public void subscriptionRemoved(SubscriptionEvent event) ;
}
