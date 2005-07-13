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

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class PatchTestDb extends TestCase {

    private AdminContext m_adminContext;
    
    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }

    public void testApplyPatch() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        m_adminContext.setPatchApplied("test-patch");
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("patch");
        assertEquals(1, actual.getRowCount());
        assertEquals("test-patch", actual.getValue(0, "name"));        
    }
}
