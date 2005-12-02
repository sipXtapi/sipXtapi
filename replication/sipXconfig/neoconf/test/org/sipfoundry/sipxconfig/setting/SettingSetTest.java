/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class SettingSetTest extends TestCase {

    public void testGetDefaultSetting() {
        SettingSet set = new SettingSet();
        set.addSetting(new SettingImpl("kuku"));
        set.addSetting(new SettingSet("bongo"));
        set.addSetting(new SettingImpl("kuku1"));

        assertEquals("kuku", set.getDefaultSetting(Setting.class).getName());
        assertEquals("bongo", set.getDefaultSetting(SettingSet.class).getName());
    }

}
