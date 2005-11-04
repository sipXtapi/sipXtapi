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

import java.io.File;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.springframework.context.ApplicationContext;

public class AutoAttendantTestDb extends SipxDatabaseTestCase {

    private DialPlanContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (DialPlanContext) appContext.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testSave() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setName("test-aa");
        aa.setDescription("aa description");
        aa.setExtension("313");
        aa.setPrompt("thankyou_goodbye.wav");
        aa.addMenuItem(DialPad.NUM_1, new AttendantMenuItem(AttendantMenuAction.GOTO_EXTENSION,
                "1234"));
        aa.addMenuItem(DialPad.NUM_5, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        m_context.storeAutoAttendant(aa);

        // attendant data
        ITable actual = TestHelper.getConnection().createDataSet().getTable("auto_attendant");
        IDataSet expectedDs = TestHelper
                .loadDataSetFlat("admin/dialplan/saveAttendantExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[auto_attendant_id]", aa.getId());
        expectedRds.addReplacementObject("[null]", null);
        ITable expected = expectedRds.getTable("auto_attendant");
        Assertion.assertEquals(actual, expected);

        // attendant menu items
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable(
                "attendant_menu_item");
        ITable expectedItems = expectedRds.getTable("attendant_menu_item");
        Assertion.assertEquals(actualItems, expectedItems);
    }

    public void testDelete() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedAttendant.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        m_context.deleteAutoAttendant(aa, "");
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable(
                "attendant_menu_item");
        assertEquals(0, actualItems.getRowCount());
    }

    public void testDeleteVxmlScript() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedAttendant.xml");
        File scriptFile = new File(TestHelper.getTestDirectory() + "/autoattendant-xcf1000.vxml");
        scriptFile.delete();

        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        VxmlGenerator vxml = new VxmlGenerator();
        vxml.setScriptsDirectory(TestHelper.getTestDirectory());
        vxml.setVelocityEngine(TestHelper.getVelocityEngine());
        vxml.generate(aa);
        assertTrue(scriptFile.exists());
        m_context.deleteAutoAttendant(aa, TestHelper.getTestDirectory());
        assertFalse(scriptFile.exists());
    }

    public void testDeleteInUse() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(2000));
        try {
            m_context.deleteAutoAttendant(aa, "");
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
            assertTrue(e.getMessage().indexOf("Internal") > 0);
        }
    }

    public void testDeleteOperatorInUse() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedOperator.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        try {
            m_context.deleteAutoAttendant(aa, "");
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
        }
    }
    
    public void testSaveNameThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedUser.xml");
        boolean gotNameInUseException = false;
        AutoAttendant aa = new AutoAttendant();
        aa.setName("alpha");
        try {
            m_context.storeAutoAttendant(aa);
        }
        catch (NameInUseException e) {
            gotNameInUseException = true;
        }
        assertTrue(gotNameInUseException);
    }
    
    public void testSaveExtensionThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedUser.xml");
        boolean gotExtensionInUseException = false;
        AutoAttendant aa = new AutoAttendant();
        aa.setName("autodafe");
        aa.setExtension("alpha");
        try {
            m_context.storeAutoAttendant(aa);
        }
        catch (ExtensionInUseException e) {
            gotExtensionInUseException = true;
        }
        assertTrue(gotExtensionInUseException);
    }

}
