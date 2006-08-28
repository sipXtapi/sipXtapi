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

        Permission permission = new Permission();
        permission.setDefaultValue(true);
        assertEquals("ENABLE", permission.getSetting().getDefaultValue());
        assertTrue(permission.getSetting().getName().startsWith("perm"));
    }

    public void testGetPrimaryKey() throws Exception {
        Permission permission = new Permission();
        Object key = permission.getPrimaryKey();
        assertEquals(-1, key);
        assertEquals("Voicemail", Permission.VOICEMAIL.getPrimaryKey());
    }
}
