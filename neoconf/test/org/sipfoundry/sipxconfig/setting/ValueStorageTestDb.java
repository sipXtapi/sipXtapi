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
import org.springframework.context.ApplicationContext;

public class ValueStorageTestDb extends TestCase {

    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple"));
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea")); 
        
        ValueStorage vs = new ValueStorage();
        SettingSet copy = (SettingSet) vs.decorate(root);
        copy.getSetting("fruit/apple").setValue("granny smith");
        copy.getSetting("vegetable/pea").setValue(null);
        
        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/SaveValueStorageExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[storage_id]", vs.getId());        
        
        ITable expected = expectedRds.getTable("setting");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testUpdate() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateValueStorageSeed.xml");        

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple"));
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea")); 
        
        ValueStorage vs = m_dao.loadValueStorage(new Integer(1));
        Setting copy = vs.decorate(root);
        copy.getSetting("fruit/apple").setValue(null);
        copy.getSetting("vegetable/pea").setValue("snow pea");
        
        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/UpdateValueStorageExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        ITable expected = expectedRds.getTable("setting");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");
        
        Assertion.assertEquals(expected, actual);        
    }   
}
