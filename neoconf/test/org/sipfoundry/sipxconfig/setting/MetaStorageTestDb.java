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

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

import junit.framework.TestCase;


public class MetaStorageTestDb extends TestCase {
    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");

        SettingGroup root = new SettingGroup();
        root.addSetting(new SettingGroup("fruit")).addSetting(new SettingImpl("apple"));
        root.addSetting(new SettingGroup("vegetable")).addSetting(new SettingImpl("pea")); 
        
        MetaStorage ms = new MetaStorage();
        SettingGroup copy = (SettingGroup) ms.decorate(root);
        copy.getSetting("fruit").getSetting("apple").setValue("granny smith");
        copy.getSetting("vegetable").getSetting("pea").setValue(null);
        
        m_dao.storeMetaStorage(ms);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/dbdata/SaveMetaStorageExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[meta_storage_id]", new Integer(ms.getId()));        
        
        ITable expected = expectedRds.getTable("setting_meta");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_meta");
        
        Assertion.assertEquals(expected, actual);
    }
}
