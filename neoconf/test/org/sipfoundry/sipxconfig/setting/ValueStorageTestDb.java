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

/**
 * @author dhubler
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ValueStorageTestDb extends TestCase {

    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    public void _testSave() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");

        SettingGroup root = new SettingGroup();
        root.addSetting(new SettingGroup("fruit")).addSetting(new Setting("apple"));
        root.addSetting(new SettingGroup("vegetable")).addSetting(new Setting("pea")); 
        
        SettingGroup copy = (SettingGroup) root.getCopy(new ValueStorage());
        copy.getSetting("fruit").getSetting("apple").setValue("granny smith");
        copy.getSetting("vegetable").getSetting("pea").setValue(null);
        

        ValueStorage vs = copy.getValueStorage();
        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/dbdata/SaveValueStorageExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[storage_id]", new Integer(vs.getId()));        
        
        ITable expected = expectedRds.getTable("setting");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testUpdate() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/dbdata/UpdateValueStorageSeed.xml");        

        SettingGroup root = new SettingGroup();
        root.addSetting(new SettingGroup("fruit")).addSetting(new Setting("apple"));
        root.addSetting(new SettingGroup("vegetable")).addSetting(new Setting("pea")); 
        
        ValueStorage vs = m_dao.loadValueStorage(1);
        SettingGroup copy = (SettingGroup) root.getCopy(vs);
        copy.getSetting("fruit").getSetting("apple").setValue(null);
        copy.getSetting("vegetable").getSetting("pea").setValue("snow pea");
        
        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/dbdata/UpdateValueStorageExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        ITable expected = expectedRds.getTable("setting");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting");
        
        Assertion.assertEquals(expected, actual);        
    }   
}
