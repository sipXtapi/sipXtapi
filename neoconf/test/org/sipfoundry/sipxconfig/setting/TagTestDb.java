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
package org.sipfoundry.sipxconfig.setting;

import java.sql.BatchUpdateException;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;
import org.springframework.dao.DataIntegrityViolationException;

public class TagTestDb extends TestCase {
    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void testGetRootTag() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        Tag root = m_dao.loadRootTag("unittest");
        IDataSet expectedDs = TestHelper
                .loadDataSetFlat("setting/GetRootTagExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[storage_id]", root.getId());
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("tag");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("tag");
        Assertion.assertEquals(expected, actual);
    }

    public void testSave() throws Throwable {
        try {
            TestHelper.cleanInsert("ClearDb.xml");

            SettingGroup root = new SettingGroup();
            root.addSetting(new SettingGroup("fruit")).addSetting(new SettingImpl("apple"));
            root.addSetting(new SettingGroup("vegetable")).addSetting(new SettingImpl("pea"));

            Tag ms = new Tag();
            ms.setResource("unittest");
            ms.setLabel("food");
            SettingGroup copy = (SettingGroup) ms.decorate(root);
            copy.getSetting("fruit").getSetting("apple").setValue("granny smith");
            copy.getSetting("vegetable").getSetting("pea").setValue(null);

            m_dao.storeTag(ms);

            IDataSet expectedDs = TestHelper
                    .loadDataSetFlat("setting/SaveTagExpected.xml");
            ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
            expectedRds.addReplacementObject("[storage_id]", ms.getId());

            ITable expected = expectedRds.getTable("setting");

            ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");

            Assertion.assertEquals(expected, actual);
        } catch (DataIntegrityViolationException e) {
            handleDiveException(e);
        }
    }

    public void testUpdate() throws Throwable {
        try {
            TestHelper.cleanInsert("ClearDb.xml");
            TestHelper.cleanInsertFlat("setting/UpdateTagSeed.xml");

            SettingGroup root = new SettingGroup();
            root.addSetting(new SettingGroup("fruit")).addSetting(new SettingImpl("apple"))
                    .setValue("granny smith");
            root.addSetting(new SettingGroup("vegetable")).addSetting(new SettingImpl("pea"))
                    .setValue("snow pea");
            root.addSetting(new SettingGroup("dairy")).addSetting(new SettingImpl("milk"));

            Tag ms = m_dao.loadTag(1);
            Setting copy = ms.decorate(root);
            // should make it disappear
            copy.getSetting("fruit").getSetting("apple").setValue("granny smith");

            // should make it update
            copy.getSetting("vegetable").getSetting("pea").setValue("snap pea");

            assertEquals(1, ms.getValues().size());
            m_dao.storeTag(ms);

            IDataSet expectedDs = TestHelper
                    .loadDataSetFlat("setting/UpdateTagExpected.xml");
            ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
            expectedRds.addReplacementObject("[null]", null);
            ITable expected = expectedRds.getTable("setting");

            ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");

            Assertion.assertEquals(expected, actual);
        } catch (DataIntegrityViolationException e) {
            handleDiveException(e);
        }
    }
    
    /**
     * TODO: Write Aspect to dump all SQL exception for BatchUpdateExceptions to stderr, this
     * would be useful to all db unittests
     */
    private void handleDiveException(DataIntegrityViolationException e) throws Throwable {
        Throwable t = e.getCause();
        if (t instanceof BatchUpdateException) {
            BatchUpdateException bue = (BatchUpdateException) t;
            bue.getNextException().printStackTrace();
        }
        throw e;
    }
}
