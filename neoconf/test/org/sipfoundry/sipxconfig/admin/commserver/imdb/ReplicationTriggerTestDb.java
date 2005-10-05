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
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class ReplicationTriggerTestDb extends SipxDatabaseTestCase {
	
	private ReplicationTrigger m_trigger;
	private SipxReplicationContext m_oldReplicationContext;
	private MockControl m_processControl;
	private SipxReplicationContext m_replicationContext;
	private SettingDao m_dao;

	protected void setUp() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        ApplicationContext app = TestHelper.getApplicationContext();
        m_trigger = (ReplicationTrigger) app.getBean("replicationTrigger");
		m_processControl = MockControl.createStrictControl(SipxReplicationContext.class);
		m_replicationContext = (SipxReplicationContext) m_processControl.getMock();
		m_replicationContext.generate(DataSet.PERMISSION);
        m_processControl.setDefaultVoidCallable();
        m_processControl.replay();
        
        m_oldReplicationContext = m_trigger.getReplicationContext();		

        m_dao = (SettingDao) app.getBean("settingDao");
	}
	
	protected void tearDown() {
        m_processControl.verify();		
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
            m_dao.saveGroup(g);            
        } finally {
            m_trigger.setReplicationContext(m_oldReplicationContext);
        }    	
    }
}
