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
}
