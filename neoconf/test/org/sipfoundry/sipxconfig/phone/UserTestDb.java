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
package org.sipfoundry.sipxconfig.phone;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;


public class UserTestDb extends TestCase {

    private PhoneContext m_context;
    
    private Class m_class = EndpointTestDb.class;
        
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);        
    }
    
    public void testLoadUserByTemplateUser() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.insertFlat("phone/dbdata/UserSearchSeed.xml");
        
        User template = new User();
        template.setDisplayId("userseed");
        List users = m_context.loadUserByTemplateUser(template);
        
        assertEquals(6, users.size());        
    }
    
}
