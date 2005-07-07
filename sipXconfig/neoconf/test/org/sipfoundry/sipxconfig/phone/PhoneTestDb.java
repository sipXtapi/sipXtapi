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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.orm.hibernate.HibernateObjectRetrievalFailureException;



/**
 * You need to call 'ant reset-db-patch' which clears a lot of data in your
 * database. before calling running this test. 
 */
public class PhoneTestDb extends TestCase {
    
    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public void testSave() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        
        Phone phone = m_context.newPhone(PolycomModel.MODEL_300.getModelId());
        PhoneData e = phone.getPhoneData();
        e.setFactoryId(PolycomModel.MODEL_300.getModelId());
        e.setSerialNumber("999123456");
        e.setName("unittest-sample phone1");
        e.setFolder(m_context.loadRootPhoneFolder());
        m_context.storePhone(phone);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("phone");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/SaveEndpointExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[phone_id_1]", e.getId());
        expectedRds.addReplacementObject("[folder_id]", e.getFolder().getId());
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("phone");
                
        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/EndpointLineSeed.xml");
        
        Phone p = m_context.loadPhone(new Integer(1));
        PhoneData e = p.getPhoneData();
        assertEquals("999123456", e.getSerialNumber());
        
        Integer id = e.getId();        
        m_context.deletePhone(p);
        try {
            m_context.loadPhone(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();        
        assertEquals(0, actual.getTable("phone").getRowCount());
        assertEquals(0, actual.getTable("line").getRowCount());
    }
    
    public void testUpdateSettings() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/EndpointSeed.xml");
        
        Phone p = m_context.loadPhone(new Integer(1));
        Setting setting = p.getSettings().getSetting("up").getSetting("headsetMode");
        String newValue = setting.getValue().equals("0") ? "1" : "0"; // toggle
        setting.setValue(newValue);
        m_context.storePhone(p);        
        m_context.flush();
        
        Phone reloadPhone = m_context.loadPhone(new Integer(1));               
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/UpdateSettingsExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        ValueStorage vs = reloadPhone.getPhoneData().getValueStorage();
        assertNotNull(vs);
        expectedRds.addReplacementObject("[storage_id]", vs.getId());

        IDataSet actual = TestHelper.getConnection().createDataSet();                
        Assertion.assertEquals(expectedRds.getTable("setting"), actual.getTable("setting"));
    }
}
