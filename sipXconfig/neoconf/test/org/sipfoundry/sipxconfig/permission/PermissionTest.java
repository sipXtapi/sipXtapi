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
package org.sipfoundry.sipxconfig.permission;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

import junit.framework.TestCase;

public class PermissionTest extends TestCase {
    public void testGetSettingPath() {
        Permission x = Permission.Type.CALL.create("x");
        assertEquals("permission/call-handling/x", x.getSettingPath());
    }
    
    public void testGetSetting() {
        Setting setting = Permission.VOICEMAIL.getSetting();
        assertEquals(Permission.VOICEMAIL.getName(), setting.getName());
        assertTrue(setting.getType() instanceof BooleanSetting);        
    }
}
