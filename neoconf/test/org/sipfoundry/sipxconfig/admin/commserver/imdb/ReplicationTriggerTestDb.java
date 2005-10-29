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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationManager;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class ReplicationTriggerTestDb extends SipxDatabaseTestCase {
	
	private ReplicationTrigger m_trigger;
	private SipxReplicationManager m_oldReplicationContext;
	private MockControl m_processControl;
	private SipxReplicationManager m_replicationContext;
	private SettingDao m_dao;

	protected void setUp() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        ApplicationContext app = TestHelper.getApplicationContext();
        m_trigger = (ReplicationTrigger) app.getBean("replicationTrigger");
		m_processControl = MockControl.createStrictControl(SipxReplicationManager.class);
        m_replicationContext = (SipxReplicationManager) m_processControl.getMock();        
        m_oldReplicationContext = m_trigger.getReplicationContext();		
        m_dao = (SettingDao) app.getBean("settingDao");
	}
    
	protected void tearDown() {
        m_processControl.verify();		
	}

    /**
     * Call this method to set up the mock control to expect exactly one call to
     * m_replicationContext.generate .
     */
    private void expectOneCallToGenerate() {
        m_replicationContext.generate(DataSet.PERMISSION);
        m_processControl.replay();        
    }

    /**
     * Call this method to set up the mock control to expect exactly one call to
     * m_replicationContext.generateAll .
     */
    private void expectOneCallToGenerateAll() {
        m_replicationContext.generateAll();
        m_processControl.replay();        
    }
    
    /**
     * Call this method to set up the mock control to expect no calls.
     */
    private void expectNoCalls() {
        m_processControl.replay();        
    }
    
    /**
     * Test that saving a user group in db actually triggers a replication
     */
    public void testNewUserGroup() throws Exception {
        try {
            m_trigger.setReplicationContext(m_replicationContext);                   
			Group g = new Group();
            g.setName("replicationTriggerTest");
            g.setResource(User.GROUP_RESOURCE_ID);
            expectOneCallToGenerate();
            m_dao.saveGroup(g);            
        } finally {
            m_trigger.setReplicationContext(m_oldReplicationContext);
        }
    }
    
    public void testUpdateUserGroup() throws Exception {    	
        TestHelper.cleanInsertFlat("admin/commserver/imdb/UserGroupSeed.db.xml");
        try {
            m_trigger.setReplicationContext(m_replicationContext);                    
            Group g = m_dao.getGroup(new Integer(1000));
            expectOneCallToGenerate();
            m_dao.saveGroup(g);            
        } finally {
            m_trigger.setReplicationContext(m_oldReplicationContext);
        }    	
    }
    
    /**
     * Test that replication happens at app startup if the replicateOnStartup
     * property is set
     */
    public void testReplicateOnStartup() throws Exception {
        try {
            m_trigger.setReplicationContext(m_replicationContext);
            m_trigger.setReplicateOnStartup(true);
            expectOneCallToGenerateAll();
            m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));
        } finally {
            m_trigger.setReplicationContext(m_oldReplicationContext);
        }       
    }
    
    /**
     * Test that replication doesn't happen at app startup if the replicateOnStartup
     * property is off
     */
    public void testNoReplicateOnStartup() throws Exception {
        try {
            m_trigger.setReplicationContext(m_replicationContext);
            m_trigger.setReplicateOnStartup(false);
            expectNoCalls();
            m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));
        } finally {
            m_trigger.setReplicationContext(m_oldReplicationContext);
        }       
    }

}
