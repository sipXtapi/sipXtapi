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
package org.sipfoundry.sipxconfig.upload;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.upload.Firmware;
import org.sipfoundry.sipxconfig.upload.FirmwareManufacturer;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class FirmwareTestDb extends SipxDatabaseTestCase {

    private UploadManager m_manager;
    
    protected void setUp() throws Exception {
        m_manager = (UploadManager) TestHelper.getApplicationContext().getBean(
                UploadManager.CONTEXT_BEAN_NAME);
    }
    
    public void testLoadSettings() throws Exception {
        Firmware f = m_manager.newFirmware(FirmwareManufacturer.getManufacturerById("polycom"));
        f.getSettings();
    }
    
    public void testUniqueUploadId() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Firmware f = m_manager.newFirmware(FirmwareManufacturer.UNMANAGED);
        f.setName("bezerk");
        assertSame(BeanWithId.UNSAVED_ID, f.getId());
        assertNotNull(f.getUniqueUploadId());
        m_manager.saveFirmware(f);
        assertEquals(f.getUniqueUploadId(), f.getId());        
    }
        
    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Firmware f = m_manager.newFirmware(FirmwareManufacturer.UNMANAGED);
        f.setName("bezerk");
        m_manager.saveFirmware(f);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("upload/SaveFirmwareExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[firmware_id]", f.getPrimaryKey());
        ITable expected = expectedRds.getTable("firmware");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("firmware");

        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/FirmwareSeed.xml");
        Firmware f = m_manager.loadFirmware(new Integer(1000));
        assertEquals("test firmware", f.getName());
        assertSame(FirmwareManufacturer.UNMANAGED, f.getManufacturer());
        
        Integer id = f.getId();        
        m_manager.deleteFirmware(f);
        try {
            m_manager.loadFirmware(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();        
        assertEquals(0, actual.getTable("firmware").getRowCount());        
    }
    
    public void testGetFirmware() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetFirmwareSeed.xml");
        Firmware[] f = (Firmware[]) m_manager.getFirmware().toArray(new Firmware[0]);
        assertEquals(2, f.length);
        assertEquals("harriot", f[0].getName());        
        assertEquals("ozzie", f[1].getName());
    }
}
