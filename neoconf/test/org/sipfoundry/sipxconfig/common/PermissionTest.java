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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class PermissionTest extends TestCase {

    public void testGetChildren() {
        Permission[] children = Permission.APPLICATION.getChildren();
        assertEquals(2, children.length);
        assertSame(Permission.SUPERADMIN, children[0]);
        assertSame(Permission.TUI_CHANGE_PIN, children[1]);
    }
}
