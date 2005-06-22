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

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;

public class GroupTestDb extends TestCase {
    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void testGetRootGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        Group root = m_dao.loadRootGroup("unittest");
        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/GetRootGroupExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[storage_id]", root.getId());
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("group_storage");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("group_storage");
        Assertion.assertEquals(expected, actual);
    }

    public void testSave() throws Throwable {
        TestHelper.cleanInsert("ClearDb.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple"));
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea"));

        Group ms = new Group();
        ms.setResource("unittest");
        ms.setName("food");
        SettingSet copy = (SettingSet) ms.decorate(root);
        copy.getSetting("fruit").getSetting("apple").setValue("granny smith");
        copy.getSetting("vegetable").getSetting("pea").setValue(null);

        m_dao.storeGroup(ms);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/SaveGroupExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[storage_id]", ms.getId());

        ITable expected = expectedRds.getTable("setting");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");

        Assertion.assertEquals(expected, actual);
    }

    public void testUpdate() throws Throwable {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateGroupSeed.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple")).setValue(
                "granny smith");
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea")).setValue(
                "snow pea");
        root.addSetting(new SettingSet("dairy")).addSetting(new SettingImpl("milk"));

        Group ms = m_dao.loadGroup(1);
        Setting copy = ms.decorate(root);
        // should make it disappear
        copy.getSetting("fruit").getSetting("apple").setValue("granny smith");

        // should make it update
        copy.getSetting("vegetable").getSetting("pea").setValue("snap pea");

        assertEquals(1, ms.getValues().size());
        m_dao.storeGroup(ms);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/UpdateGroupExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        ITable expected = expectedRds.getTable("setting");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");

        Assertion.assertEquals(expected, actual);
    }

    public void testDuplicateName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateGroupSeed.xml");
        
        Group ms = m_dao.loadGroup(1);
        Group duplicate = new Group();
        duplicate.setName(ms.getName());
        duplicate.setResource(ms.getResource());
        
        try {
            m_dao.storeGroup(duplicate);
            fail();
        } catch (UserException u) {            
            assertTrue(true);
        }
    }
}
