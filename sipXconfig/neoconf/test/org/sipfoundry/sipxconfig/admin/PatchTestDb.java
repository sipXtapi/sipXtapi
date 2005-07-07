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
package org.sipfoundry.sipxconfig.admin;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class PatchTestDb extends TestCase {

    private AdminContext m_adminContext;
    
    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }
    
    public void testPatchRequired() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("admin/seedPatchRequired.xml");
        
        try {
            m_adminContext.requirePatch(new Integer(2000));
            fail();
        } catch (PatchNotAppliedException expected) {
            assertTrue(true);
        }        
    }
}
