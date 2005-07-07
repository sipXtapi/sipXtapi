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

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;

public class BackupPlanTestDb extends TestCase {

    private AdminContext m_adminContext;
    
    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }
    
    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        
        BackupPlan plan = new BackupPlan();
        m_adminContext.storeBackupPlan(plan);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("backup_plan");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("admin/SaveBackupPlanExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[backup_plan_id]", plan.getId());
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("backup_plan");
                
        Assertion.assertEquals(expected, actual);        
    }
}
