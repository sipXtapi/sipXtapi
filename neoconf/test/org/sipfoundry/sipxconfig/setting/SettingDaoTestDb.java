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
        SettingMap map = new SettingMap();
        SettingValue value = new SettingValue("/any/path", "value");
        map.put(value.getPath(), value);
        ApplicationContext context = TestHelper.getApplicationContext();
        SettingDao dao = (SettingDao) context.getBean("settingDao");
        dao.storeSettingValues(map);
        
        TestHelper.deleteOnTearDown(map);
    }
}
