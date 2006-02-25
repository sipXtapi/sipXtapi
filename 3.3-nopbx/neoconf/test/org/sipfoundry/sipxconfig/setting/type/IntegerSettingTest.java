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

public class IntegerSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new IntegerSetting();
        assertEquals(new Integer(51), type.convertToTypedValue("51"));
        assertNull(type.convertToTypedValue("kuku"));
    }

    public void testConvertToStringValue() {
        SettingType type = new IntegerSetting();
        assertEquals("52", type.convertToStringValue(new Integer(52)));
        assertNull(type.convertToStringValue(null));
    }
}
