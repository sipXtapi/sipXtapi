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

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;


public class UserTestDb extends TestCase {

    private CoreContext m_core;
    
    protected void setUp() throws Exception {
        m_core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
    }
    
    public void testLoadUserByTemplateUser() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.insertFlat("common/UserSearchSeed.xml");
        
        User template = new User();
        template.setDisplayId("userseed");
        List users = m_core.loadUserByTemplateUser(template);
        
        assertEquals(6, users.size());        
    }
    
    public void testSave() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        User user = new User();
        user.setDisplayId("userid");
        user.setFirstName("FirstName");
        user.setLastName("LastName");
        user.setPassword("password");
        user.setExtension("1234");
        user.setOrganization(m_core.loadRootOrganization());
        m_core.saveUser(user);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("common/SaveUserExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[user_id]", user.getId());        
        expectedRds.addReplacementObject("[null]", null);        
        
        ITable expected = expectedRds.getTable("users");                
        ITable actual = TestHelper.getConnection().createQueryTable("users", "select * from users where display_id='userid'");
        
        Assertion.assertEquals(expected, actual);                
        
    }
}
