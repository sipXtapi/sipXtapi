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
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.springframework.context.ApplicationContext;

public class AutoAttendantTestDb extends TestCase {
    
    private DialPlanContext m_plan;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_plan = (DialPlanContext) appContext
                .getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
    }
    
    public void testSave() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setName("test-aa");
        aa.setDescription("aa description");
        aa.setExtension("313");
        aa.setPrompt("thankyou_goodbye.wav");
        aa.addMenuItem(DialPad.NUM_1, new AttendantMenuItem(AttendantMenuAction.GOTO_EXTENSION, "1234"));
        aa.addMenuItem(DialPad.NUM_5, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        m_plan.storeAutoAttendant(aa);
        
        // attendant data
        ITable actual = TestHelper.getConnection().createDataSet().getTable("auto_attendant");
        IDataSet expectedDs = TestHelper.loadDataSetFlat("admin/dialplan/dbdata/saveAttendantExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[auto_attendant_id]", aa.getId());        
        expectedRds.addReplacementObject("[null]", null);        
        ITable expected = expectedRds.getTable("auto_attendant");                
        Assertion.assertEquals(actual, expected);
        
        // attendant menu items
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_menu_item");
        ITable expectedItems = expectedRds.getTable("attendant_menu_item");                
        Assertion.assertEquals(actualItems, expectedItems);        
    }
}
