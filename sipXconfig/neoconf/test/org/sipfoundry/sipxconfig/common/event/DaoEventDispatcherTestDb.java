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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class DaoEventDispatcherTestDb extends SipxDatabaseTestCase {
    
    private DaoEventDispatcher m_dispatcher;
    private DaoEventPublisher m_publisherOrg;
    private CoreManager m_core;


    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_core = (CoreManager) app.getBean(CoreManager.CONTEXT_BEAN_NAME);
        m_dispatcher = (DaoEventDispatcher)app.getBean("onSaveEventDispatcher");
        TestHelper.cleanInsert("ClearDb.xml");
        m_publisherOrg = null;
    }
    
    protected void tearDown() throws Exception {
        // this is called in finally block to restore the appliation context
        if(m_publisherOrg != null) {            
            m_dispatcher.setPublisher(m_publisherOrg);
        }
    }
        	
    public void testOnSaveAspect() throws Exception {
    	User u = new User();
    	u.setUserName("testme");
    	
    	MockControl publisherCtrl = MockControl.createControl(DaoEventPublisher.class);
		DaoEventPublisher publisher = (DaoEventPublisher) publisherCtrl.getMock();
		publisher.publishSave(u);
        publisherCtrl.replay();

        m_publisherOrg = m_dispatcher.getPublisher();
        m_dispatcher.setPublisher(publisher);
        
        // should generate event
    	m_core.saveUser(u);
        
        // no event now
    	m_core.clear();
    	
    	publisherCtrl.verify();
    }
}


