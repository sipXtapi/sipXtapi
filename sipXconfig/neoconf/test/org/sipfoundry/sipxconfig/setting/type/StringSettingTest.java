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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.setting.Setting;

import junit.framework.TestCase;

public class StringSettingTest extends TestCase {

    public void testConvertToTypedValue() {
        SettingType type = new StringSetting();
        assertEquals("", type.convertToTypedValue(""));
        assertEquals("bongo", type.convertToTypedValue("bongo"));
        assertNull(type.convertToTypedValue(null));
    }

    public void testConvertToStringValue() {
        SettingType type = new StringSetting();
        assertEquals("bongo", type.convertToStringValue("bongo"));
        assertNull("Only null is null", type.convertToStringValue(null));
        assertEquals("", type.convertToStringValue(""));
        assertEquals("\t ", type.convertToStringValue("\t "));
        assertNull(type.convertToStringValue(null));
    }

    public void testGetDefaultValueForPassword() {
        StringSetting type = new StringSetting();
        type.setPassword(true);
        
        MockControl settingCtrl = MockControl.createStrictControl(Setting.class);
        settingCtrl.replay();
        
        assertNull(type.getLabel("foo"));

        settingCtrl.verify();
    }    
}
