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
package org.sipfoundry.sipxconfig.legacy;

import java.sql.SQLException;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class LegacyContextImplTestDb extends TestCase {

    private LegacyContext m_legacyContext;
    private CoreContext m_coreContext;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_legacyContext = (LegacyContext) appContext.getBean(LegacyContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        try {
            TestHelper.cleanInsert("dbdata/ClearDb.xml");

        } catch (SQLException e) {
            System.err.println(e.getNextException().getMessage());
            throw e;
        }
    }
    
    public void testGetConfigSetsForUser() {
        User user = m_coreContext.loadUser(4);
        UserConfigSet cs = m_legacyContext.getConfigSetForUser(user);
        assertEquals("<PROFILE></PROFILE>", cs.getContent());            
    }
        
    public void testCheckUserPermission() {
        User user = m_coreContext.loadUser(4);
        assertFalse(m_legacyContext.checkUserPermission(user,Permission.VOICEMAIL));
    }
}
