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
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class InitializationTaskTestDb extends SipxDatabaseTestCase {

    private AdminManager m_adminContext;
    
    protected void setUp() throws Exception {
        m_adminContext = (AdminManager) TestHelper.getApplicationContext().getBean(
                AdminManager.CONTEXT_BEAN_NAME);
    }

    public void testDeleteInitializationTask() throws Exception {
        TestHelper.cleanInsertFlat("admin/InitializationTaskSeed.xml");
        
        m_adminContext.deleteInitializationTask("test-task");
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("initialization_task");
        assertEquals(0, actual.getRowCount());
    }

    public void testGetInitializationTask() throws Exception {
        TestHelper.cleanInsertFlat("admin/InitializationTaskSeed.xml");
        
        String[] tasks = m_adminContext.getInitializationTasks();
        assertEquals(1, tasks.length);
        assertEquals("test-task", tasks[0]);
    }
}
