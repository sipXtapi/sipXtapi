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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SettingDaoTestDb extends TestCase {
    
    public void tearDown() throws Exception {
        TestHelper.tearDown();
    }
    
    public void testSettingGroup() {
        SettingGroup root = new SettingGroup();
        root.addSetting(new Setting("setting"));
        
        SettingGroup copy = (SettingGroup) root.getCopy(new ValueStorage());
        copy.getSetting("setting").setValue("some value");

        ApplicationContext context = TestHelper.getApplicationContext();
        SettingDao dao = (SettingDao) context.getBean("settingDao");
        dao.storeValueStorage(copy.getValueStorage());
        
        TestHelper.deleteOnTearDown(copy.getValueStorage());
    }
}
