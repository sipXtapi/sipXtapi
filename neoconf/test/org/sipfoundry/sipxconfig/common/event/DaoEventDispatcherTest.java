/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.User;

public class DaoEventDispatcherTest extends TestCase {
	
	public void testOnDelete() throws Throwable {
    	User u = new User();
    	u.setUserName("testme");

    	MockControl publisherCtrl = MockControl.createControl(DaoEventPublisher.class);
		DaoEventPublisher publisher = (DaoEventPublisher) publisherCtrl.getMock();
		publisher.publishDelete(u);
		publisherCtrl.replay();
		
		DaoEventDispatcher dispatcher = DaoEventDispatcher.createDeleteDispatcher();
        dispatcher.setPublisher(publisher);
		dispatcher.before(null, new Object[] {u}, null);
		
		publisherCtrl.verify();		
	}

    public void testOnSave() throws Throwable {
        User u = new User();
        u.setUserName("testme");

        MockControl publisherCtrl = MockControl.createControl(DaoEventPublisher.class);
        DaoEventPublisher publisher = (DaoEventPublisher) publisherCtrl.getMock();
        publisher.publishSave(u);
        publisherCtrl.replay();
        
        DaoEventDispatcher dispatcher = DaoEventDispatcher.createSaveDispatcher();
        dispatcher.setPublisher(publisher);
        dispatcher.before(null, new Object[] {u}, null);
        
        publisherCtrl.verify();     
    }    
}


