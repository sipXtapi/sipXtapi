/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

import junit.framework.TestCase;

public class RealSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new RealSetting();
        assertEquals(new Double(51), type.convertToTypedValue("51"));
        assertNull(type.convertToTypedValue("kuku"));
    }

    public void testConvertToStringValue() {
        SettingType type = new RealSetting();
        assertEquals("52.51", type.convertToStringValue(new Float(52.51)));
        assertNull(type.convertToStringValue(null));
    }
}
