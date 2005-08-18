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

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class ReplicationTriggerTestDb extends TestCase {
    
    /**
     * Test that saving a user group in db actually triggers a replication
     */
    public void testSaveUserGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        ApplicationContext app = TestHelper.getApplicationContext();
        ReplicationTrigger trigger = (ReplicationTrigger) app.getBean("replicationTrigger");
        
        MockControl processControl = MockControl.createStrictControl(SipxProcessContext.class);
        SipxProcessContext processContext = (SipxProcessContext) processControl.getMock();
        processContext.generate(DataSet.PERMISSION);
        processControl.setDefaultVoidCallable();
        processControl.replay();
        
        SipxProcessContext oldProcessContext = trigger.getProcessContext();
        try {
            trigger.setProcessContext(processContext);        
            
            SettingDao dao = (SettingDao) app.getBean("settingDao");
            Group g = new Group();
            g.setName("replicationTriggerTest");
            g.setResource(User.GROUP_RESOURCE_ID);
            dao.storeGroup(g);            
        } finally {
            trigger.setProcessContext(oldProcessContext);
        }

        processControl.verify();
    }
}
