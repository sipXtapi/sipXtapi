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

import org.sipfoundry.sipxconfig.phone.Setting;

import junit.framework.TestCase;

/**
 * Comments
 */
public class SettingTest extends TestCase {
    
    public void testSetting() {
        Object value = new Object();
        Setting s = new Setting("setting", value);
        assertEquals(value, s.getValue());
        
        /*
        try {
            s.addSetting(new Setting());
            fail("Should not be able to set setting on leaf node");
        }
        catch (IllegalArgumentException expected) {
            assertEquals(0, s.getSettings().size());
        }

        try {
            s.getSetting("foobar");
            fail("Settings should not be gettable on leaf node");
        }
        catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
        */        
    }
}
