/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import org.sipfoundry.sipxconfig.setting.SettingImpl;

import junit.framework.TestCase;

public class SettingPermissionTest extends TestCase {

    public void testGetPrimaryKey() throws Exception {
        SettingImpl setting = new SettingImpl();
        setting.setName("voicemail");
        setting.setLabel("Voicemail");
        setting.setDescription("Voicemail Permissions");
        setting.setValue("DISABLED");

        SettingPermission permission = new SettingPermission(setting);

        assertEquals("voicemail", permission.getPrimaryKey());
        assertEquals("Voicemail", permission.getLabel());
        assertEquals("Voicemail Permissions", permission.getDescription());
        assertFalse(permission.getDefaultValue());
    }
}
