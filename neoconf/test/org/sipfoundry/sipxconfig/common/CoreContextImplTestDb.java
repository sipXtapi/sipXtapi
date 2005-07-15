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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CoreContextImplTestDb extends TestCase {

    private CoreContext core;

    protected void setUp() throws Exception {
        core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testSearchByDisplayId() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setDisplayId("userseed");
        List users = core.loadUserByTemplateUser(template);

        assertEquals(6, users.size());
    }

    public void testSearchFormBlank() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User template = new User();
        List users = core.loadUserByTemplateUser(template);

        assertEquals(9, users.size());
    }

    public void testLoadUsers() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        List users = core.loadUsers();

        assertEquals(9, users.size());
    }
    
    public void testLoadGroups() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/UserGroupSeed.xml");
        List groups = core.getUserGroups(); 
        assertEquals(1, groups.size());
        Group group = (Group) groups.get(0);
        assertEquals("SeedUserGroup1", group.getName());
    }
    
    public void testGetUserSettingModel() {
        Setting model = core.getUserSettingsModel();
        assertNotNull(model.getSetting("permission"));
    }

    public void testAliases() throws Exception {
        List userAliases = core.getUserAliases();
        assertEquals(0, userAliases.size());

        TestHelper.cleanInsertFlat("common/TestUserSeed.xml");

        userAliases = core.getUserAliases();
        assertEquals(1, userAliases.size());
    }
}
