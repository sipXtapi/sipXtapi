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
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

import junit.framework.TestCase;

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
        Collection configSetsForUser = m_legacyContext.getConfigSetsForUser(user);
        assertEquals(1,configSetsForUser.size());
        for (Iterator i = configSetsForUser.iterator(); i.hasNext();) {
            UserConfigSet cs = (UserConfigSet) i.next();
            assertEquals("<PROFILE></PROFILE>", cs.getContent());            
        }
    }
}
