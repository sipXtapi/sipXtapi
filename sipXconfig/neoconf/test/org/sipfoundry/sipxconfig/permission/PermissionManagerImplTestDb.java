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
package org.sipfoundry.sipxconfig.permission;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class PermissionManagerImplTestDb extends SipxDatabaseTestCase {

    private PermissionManager m_manager;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_manager = (PermissionManager) app.getBean(PermissionManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetPermission() throws Exception {
        Permission permission = m_manager.getPermission(Permission.VOICEMAIL.getName());
        assertEquals(Permission.VOICEMAIL, permission);

        TestHelper.insertFlat("permission/permission.db.xml");

        permission = m_manager.getPermission("kuku");
        assertNotNull(permission);

        assertEquals("kukuDescription", permission.getDescription());
        assertEquals("kukuLabel", permission.getLabel());

        Permission permissionNull = m_manager.getPermission("nonexistent");
        assertNull(permissionNull);
    }

    public void testAddCallPermission() throws Exception {
        Permission permission = new Permission();
        permission.setName("abc");
        permission.setDescription("description");
        permission.setLabel("label");

        m_manager.addCallPermission(permission);

        assertEquals(1, getConnection().getRowCount("permission", "where name = 'abc'"));
    }

    public void testGetCallPermissions() throws Exception {
        Collection<Permission> permissions = m_manager.getCallPermissions();
        int size = permissions.size();

        TestHelper.insertFlat("permission/permission.db.xml");
        permissions = m_manager.getCallPermissions();
        assertEquals(size + 2, permissions.size());
    }

    public void testRemoveCallPermissions() throws Exception {
        String[] names = {
            "kuku", "bongo"
        };
        TestHelper.insertFlat("permission/permission.db.xml");

        m_manager.removeCallPermissions(Arrays.asList(names));
        assertEquals(0, getConnection().getRowCount("permission"));
    }

    public void testRemoveCallBuiltInPermissions() throws Exception {
        m_manager.removeCallPermissions(Collections.singleton(Permission.VOICEMAIL.getName()));
        assertEquals(0, getConnection().getRowCount("permission"));
    }
}
