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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

public class UserCallerAliasInfoTest extends TestCase {
    private User m_user;

    protected void setUp() throws Exception {
        m_user = new User();
        PermissionManagerImpl impl = new PermissionManagerImpl();
        impl.setModelFilesContext(TestHelper.getModelFilesContext());
        m_user.setPermissionManager(impl);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetExternalNumber() {
        m_user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, "333");
        UserCallerAliasInfo info = new UserCallerAliasInfo(m_user);
        assertEquals("333", info.getExternalNumber());
        assertFalse(info.isAnonymous());

        m_user.setSettingValue(UserCallerAliasInfo.ANONYMOUS_CALLER_ALIAS, "1");
        info = new UserCallerAliasInfo(m_user);
        assertTrue(info.isAnonymous());
    }
}
