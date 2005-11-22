/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class FirmwareTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_phoneContext;
    
    protected void setUp() throws Exception {
        m_phoneContext = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
        
    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Firmware f = new Firmware();
        f.setName("bezerk");
        f.setManufacturer(FirmwareManufacturer.UNMANAGED);
        f.setDeliveryId("tftp");
        m_phoneContext.saveFirmware(f);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/SaveFirmwareExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[firmware_id]", f.getPrimaryKey());
        ITable expected = expectedRds.getTable("firmware");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("firmware");

        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/FirmwareSeed.xml");
        Firmware f = m_phoneContext.loadFirmware(new Integer(1000));
        assertEquals("test firmware", f.getName());
        assertSame(FirmwareManufacturer.UNMANAGED, f.getManufacturer());
        
        Integer id = f.getId();        
        m_phoneContext.deleteFirmware(f);
        try {
            m_phoneContext.loadFirmware(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();        
        assertEquals(0, actual.getTable("firmware").getRowCount());        
    }
}
