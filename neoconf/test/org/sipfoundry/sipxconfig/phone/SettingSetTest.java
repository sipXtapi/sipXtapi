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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

/**
 * Comments
 */
public class SettingSetTest extends TestCase {
    
    public void testIterator() {
        SettingSet root = new SettingSet();
        SettingSet si = new SettingSet();
        root.addSetting(si);
        for (int i = 0; i < 10; i++) {
            si.addSetting(new Setting("setting-" + i, new Integer(i)));
            SettingSet sj = new SettingSet("settings-" + i);
            si.addSetting(sj);
            for (int j = 0; j < 10; j++) {
                sj.addSetting(new Setting("settings-" + i + "-" + j, new Integer(i * j)));                
            }
        }
    }
}
