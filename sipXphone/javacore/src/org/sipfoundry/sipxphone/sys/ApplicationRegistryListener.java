/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/ApplicationRegistryListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys ;

/**
 * Listener callback decloration for the application registry.  Subscribers
 * are notified whenever an application is added/remove from the application
 * registry. 
 *
 * NOTE: On successful adds/removals will make this list.  Applications that 
 *       fail to load will not fire off applicationAdded lists, even though 
 *       that application is in the load list.
 * 
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface ApplicationRegistryListener
{
    /**
     * Invoked whenever an application is added to the application registry
     */
    public void applicationAdded(ApplicationDescriptor desc) ;
    
    
    /**
     * Invoked whenever an application is remove from the application registry
     */    
    public void applicationRemoved(ApplicationDescriptor desc) ;        
}
