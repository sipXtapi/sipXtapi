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

import java.util.List;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.orm.hibernate.HibernateObjectRetrievalFailureException;


public class PhoneTestDb extends TestCase {
    
    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        
        Phone phone = m_context.newPhone(PolycomModel.MODEL_300.getModelId());
        PhoneData e = phone.getPhoneData();
        e.setFactoryId(PolycomModel.MODEL_300.getModelId());
        e.setSerialNumber("999123456");
        e.setName("unittest-sample phone1");
        m_context.storePhone(phone);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("phone");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/SaveEndpointExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[phone_id_1]", e.getId());
        expectedRds.addReplacementObject("[null]", null);                      
        ITable expected = expectedRds.getTable("phone");                
        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        
        Phone p = m_context.loadPhone(new Integer(1000));
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
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        
        Phone p = m_context.loadPhone(new Integer(1000));
        Setting setting = p.getSettings().getSetting("up/headsetMode");
        String newValue = setting.getValue().equals("0") ? "1" : "0"; // toggle
        setting.setValue(newValue);
        m_context.storePhone(p);        
        m_context.flush();
        
        Phone reloadPhone = m_context.loadPhone(new Integer(1000));               
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/UpdateSettingsExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);        

        ValueStorage s = reloadPhone.getPhoneData().getValueStorage();
        assertNotNull(s);
        expectedRds.addReplacementObject("[storage_id]", s.getId());

        IDataSet actual = TestHelper.getConnection().createDataSet();                
        Assertion.assertEquals(expectedRds.getTable("setting"), actual.getTable("setting"));
    }
    
    public void testAddGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        TestHelper.cleanInsertFlat("phone/SeedPhoneGroup.xml");
        
        Phone p = m_context.loadPhone(new Integer(1000));
        List groups = m_context.getGroupsWithoutRoot();
        p.getPhoneData().addGroup((Group) groups.get(0));
        m_context.storePhone(p);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddGroupExpected.xml");
        IDataSet actual = TestHelper.getConnection().createDataSet();                
        Assertion.assertEquals(expectedDs.getTable("phone_group"), actual.getTable("phone_group"));        
    }
    
    public void testRemoveGroupThenAddBackThenAddAnotherGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        TestHelper.cleanInsertFlat("phone/SeedPhoneGroup.xml");
        
        Phone p = m_context.loadPhone(new Integer(1000));
        List groups = m_context.getGroupsWithoutRoot();
        p.getPhoneData().addGroup((Group) groups.get(0));
        m_context.storePhone(p);
        p = null;
        
        Phone reloaded = m_context.loadPhone(new Integer(1000));
        reloaded.getPhoneData().getGroups().clear();
        reloaded.getPhoneData().addGroup((Group) groups.get(0));
        reloaded.getPhoneData().addGroup((Group) groups.get(1));
        m_context.storePhone(reloaded);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddSecondGroupExpected.xml");
        IDataSet actual = TestHelper.getConnection().createDataSet();                
        Assertion.assertEquals(expectedDs.getTable("phone_group"), actual.getTable("phone_group"));        
    }
}
