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
        f.setManufacturerId("unmanagedPhone");
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
}
