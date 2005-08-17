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
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PermissionReplicationTriggerTestDb extends TestCase {

    private CoreContext core;

    private SettingDao setting;

    protected void setUp() throws Exception {
        core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
        setting = (SettingDao) TestHelper.getApplicationContext().getBean(
                SettingDao.CONTEXT_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    /**
     * Tests that PermissionReplicationTrigger is actually called when user group is saved
     */
    public void testWhenUserGroupIsUpdatedThenPermissionsReplicate() throws Exception {
        MockControl processControl = MockControl.createStrictControl(SipxProcessContext.class);
        SipxProcessContext processContext = (SipxProcessContext) processControl.getMock();
        processContext.generate(DataSet.PERMISSION);
        processControl.setDefaultVoidCallable();
        processControl.replay();
        
        SipxProcessContext oldContext = core.getProcessContext();
        try {
            core.setProcessContext(processContext);
            
            TestHelper.cleanInsert("ClearDb.xml");
            TestHelper.insertFlat("common/UserGroupSeed.xml");
            List groups = core.getUserGroups(); 
            assertEquals(1, groups.size());
            Group group = (Group) groups.get(0);
            setting.storeGroup(group);
            
            processControl.verify();            
        } finally {
            core.setProcessContext(oldContext);
        }
    }
}
