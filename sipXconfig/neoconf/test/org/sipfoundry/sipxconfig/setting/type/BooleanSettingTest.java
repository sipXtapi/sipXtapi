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

import java.util.Locale;
import java.util.ResourceBundle;

import junit.framework.TestCase;

public class BooleanSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new BooleanSetting();
        assertTrue(((Boolean) type.convertToTypedValue("1")).booleanValue());
        assertFalse(((Boolean) type.convertToTypedValue("0")).booleanValue());
        assertNull(type.convertToTypedValue(null));
        assertFalse(((Boolean) type.convertToTypedValue("xyz")).booleanValue());
    }

    public void testConvertToStringValue() {
        BooleanSetting type = new BooleanSetting();
        type.setTrueValue("enabled");
        type.setFalseValue("disabled");
        assertEquals("enabled", type.convertToStringValue(Boolean.TRUE));
        assertEquals("disabled", type.convertToStringValue(Boolean.FALSE));
        assertNull(type.convertToStringValue(null));
    }
    
    public void testGetLabel() {
        BooleanSetting type = new BooleanSetting();
        ResourceBundle rb = ResourceBundle.getBundle(BooleanSetting.class.getName(), Locale.US);
        assertEquals("checked", type.getResourceLabel(rb, "1"));
        assertEquals("unchecked", type.getResourceLabel(rb, "0"));
        assertNull(type.getResourceLabel(rb, null));
    }
}
