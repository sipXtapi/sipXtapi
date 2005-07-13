/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class CoreContextImplTestDb extends TestCase {

    private CoreContext m_core;
    
    protected void setUp() throws Exception {
        m_core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
    }

    public void testSearchByDisplayId() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setDisplayId("userseed");
        List users = m_core.loadUserByTemplateUser(template);

        assertEquals(6, users.size());
    }

    public void testSearchFormBlank() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User template = new User();
        List users = m_core.loadUserByTemplateUser(template);

        assertEquals(9, users.size());
    }
    
    public void testLoadUsers() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");
        
        List users = m_core.loadUsers();
        
        assertEquals(9, users.size());
    }
}
