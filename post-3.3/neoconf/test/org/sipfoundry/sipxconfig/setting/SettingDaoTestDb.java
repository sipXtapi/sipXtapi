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
package org.sipfoundry.sipxconfig.setting;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SettingDaoTestDb extends SipxDatabaseTestCase {
    
    SettingDao dao;
    
    protected void setUp() {
        ApplicationContext context = TestHelper.getApplicationContext();
        dao = (SettingDao) context.getBean("settingDao");        
    }
    
    public void testSettingGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingImpl("setting"));
        
        ValueStorage vs = new ValueStorage();
        vs.decorate(root);
        root.getSetting("setting").setValue("some value");

        dao.storeValueStorage(vs);
    }
    
    public void testGetGroupsAsString() {
        Group[] groups = new Group[2];
        groups[0] = new Group();
        groups[0].setName("g1");
        groups[1] = new Group();
        groups[1].setName("g2");
        assertEquals("g1 g2", dao.getGroupsAsString(Arrays.asList(groups)));
    }
    
    public void testGetGroupsByString() throws Exception {
        Group[] groups = (Group[]) dao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ").toArray(new Group[0]);
        assertEquals(5, groups.length);
        assertEquals("g0", groups[0].getName());
        assertEquals("g4", groups[4].getName());        
    }
    
    public void testGetGroupsByEmptyString() throws Exception {
        assertEquals(0, dao.getGroupsByString("foo", "").size());
        assertEquals(0, dao.getGroupsByString("foo", "  ").size());
        assertEquals(0, dao.getGroupsByString("foo", null).size());
    }
    
}
