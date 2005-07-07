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

public class FolderTestDb extends TestCase {
    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void testGetRootFolder() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");

        Folder root = m_dao.loadRootFolder("unittest");
        IDataSet expectedDs = TestHelper
                .loadDataSetFlat("setting/dbdata/GetRootFolderExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[folder_id]", root.getId());
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("folder");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("folder");
        Assertion.assertEquals(expected, actual);
    }

    public void testSave() throws Throwable {
        try {
            TestHelper.cleanInsert("dbdata/ClearDb.xml");

            SettingGroup root = new SettingGroup();
            root.addSetting(new SettingGroup("fruit")).addSetting(new SettingImpl("apple"));
            root.addSetting(new SettingGroup("vegetable")).addSetting(new SettingImpl("pea"));

            Folder ms = new Folder();
            ms.setResource("unittest");
            ms.setLabel("food");
            SettingGroup copy = (SettingGroup) ms.decorate(root);
            copy.getSetting("fruit").getSetting("apple").setValue("granny smith");
            copy.getSetting("vegetable").getSetting("pea").setValue(null);

            m_dao.storeFolder(ms);

            IDataSet expectedDs = TestHelper
                    .loadDataSetFlat("setting/dbdata/SaveFolderExpected.xml");
            ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
            expectedRds.addReplacementObject("[folder_id]", ms.getId());

            ITable expected = expectedRds.getTable("folder_setting");

            ITable actual = TestHelper.getConnection().createDataSet().getTable("folder_setting");

            Assertion.assertEquals(expected, actual);
        } catch (DataIntegrityViolationException e) {
            handleDiveException(e);
        }
    }

    public void testUpdate() throws Throwable {
        try {
            TestHelper.cleanInsert("dbdata/ClearDb.xml");
            TestHelper.cleanInsertFlat("setting/dbdata/UpdateFolderSeed.xml");

            SettingGroup root = new SettingGroup();
            root.addSetting(new SettingGroup("fruit")).addSetting(new SettingImpl("apple"))
                    .setValue("granny smith");
            root.addSetting(new SettingGroup("vegetable")).addSetting(new SettingImpl("pea"))
                    .setValue("snow pea");
            root.addSetting(new SettingGroup("dairy")).addSetting(new SettingImpl("milk"));

            Folder ms = m_dao.loadFolder(1);
            Setting copy = ms.decorate(root);
            // should make it disappear
            copy.getSetting("fruit").getSetting("apple").setValue("granny smith");

            // should make it update
            copy.getSetting("vegetable").getSetting("pea").setValue("snap pea");

            // should make it new
            copy.getSetting("dairy").getSetting("milk").setHidden(true);

            assertEquals(2, ms.getFolderSettings().size());
            m_dao.storeFolder(ms);

            IDataSet expectedDs = TestHelper
                    .loadDataSetFlat("setting/dbdata/UpdateFolderExpected.xml");
            ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
            expectedRds.addReplacementObject("[null]", null);
            ITable expected = expectedRds.getTable("folder_setting");

            ITable actual = TestHelper.getConnection().createDataSet().getTable("folder_setting");

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