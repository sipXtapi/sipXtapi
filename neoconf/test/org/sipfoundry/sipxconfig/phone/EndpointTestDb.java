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
import org.sipfoundry.sipxconfig.phone.polycom.Polycom;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.orm.hibernate.HibernateObjectRetrievalFailureException;



/**
 * You need to call 'ant reset-db-patch' which clears a lot of data in your
 * database. before calling running this test. 
 */
public class EndpointTestDb extends TestCase {
    
    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public void testSave() throws Exception {        
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        
        Endpoint e = new Endpoint();
        e.setPhoneId(Polycom.MODEL_300.getModelId());
        e.setSerialNumber("999123456");
        e.setName("unittest-sample phone1");
        e.setFolder(m_context.loadRootEndpointFolder());
        m_context.storeEndpoint(e);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("endpoint");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/SaveEndpointExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[endpoint_id_1]", new Integer(e.getId()));
        expectedRds.addReplacementObject("[folder_id]", new Integer(e.getFolder().getId()));
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("endpoint");
                
        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsertFlat("phone/dbdata/EndpointSeed.xml");
        
        Endpoint e = m_context.loadEndpoint(1);
        assertEquals("999123456", e.getSerialNumber());
        
        int id = e.getId();        
        m_context.deleteEndpoint(e);
        try {
            m_context.loadEndpoint(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }
    }

}
