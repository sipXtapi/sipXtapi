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

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl.PermissionCreator;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PermissionManagerImplTest extends TestCase {
    public void testVisitorExistingSetting() {
        Permission noAccess = new Permission(Permission.Type.CALL, "NoAccess");

        PermissionManager manager = EasyMock.createMock(PermissionManager.class);
        EasyMock.expect(manager.getPermission("NoAccess")).andReturn(noAccess);

        Setting setting = EasyMock.createMock(Setting.class);
        EasyMock.expect(setting.getName()).andReturn("NoAccess");
        EasyMock.expect(setting.getDescription()).andReturn("NoAccessDescription");
        EasyMock.expect(setting.getLabel()).andReturn("NoAccessLabel");

        EasyMock.replay(setting, manager);

        PermissionCreator creator = new PermissionManagerImpl.PermissionCreator(manager);
        creator.visitSetting(setting);

        assertEquals("NoAccessLabel", noAccess.getLabel());
        assertEquals("NoAccessDescription", noAccess.getDescription());

        EasyMock.verify(setting, manager);
    }

    public void testVisitorNewSetting() {
        Permission p = new Permission(Permission.Type.CALL, "bongoPerm");

        PermissionManager manager = EasyMock.createMock(PermissionManager.class);
        EasyMock.expect(manager.getPermission("bongoPerm")).andReturn(null);
        manager.addCallPermission(p);
        EasyMock.expectLastCall();

        Setting setting = EasyMock.createMock(Setting.class);

        EasyMock.expect(setting.getName()).andReturn("bongoPerm");
        EasyMock.expect(setting.getDescription()).andReturn("bongoPermDescription");
        EasyMock.expect(setting.getLabel()).andReturn("bongoPermLabel");

        EasyMock.replay(setting, manager);

        PermissionCreator creator = new PermissionManagerImpl.PermissionCreator(manager);
        creator.visitSetting(setting);

        assertNotNull(p);

        EasyMock.verify(setting, manager);
    }

}
