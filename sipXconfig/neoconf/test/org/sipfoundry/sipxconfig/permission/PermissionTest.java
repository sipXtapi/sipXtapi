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

import junit.framework.TestCase;

public class PermissionTest extends TestCase {
    public void testGetSettingPath() {
        Permission x = Permission.Type.CALL.create("x");
        assertEquals("permission/call-handling/x", x.getSettingPath());
    }
}
