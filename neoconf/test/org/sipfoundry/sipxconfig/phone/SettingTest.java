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
public class SettingTest extends TestCase {
    
    public void testSetting() {
        Object value = new Object();
        Setting s = new Setting("setting", value);
        assertEquals(value, s.getValue());
    }
}
