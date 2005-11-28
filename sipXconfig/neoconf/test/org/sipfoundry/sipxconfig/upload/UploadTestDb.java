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
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class UploadTestDb extends SipxDatabaseTestCase {

    private UploadManager m_manager;
    
    protected void setUp() throws Exception {
        m_manager = (UploadManager) TestHelper.getApplicationContext().getBean(
                UploadManager.CONTEXT_BEAN_NAME);
    }
    
    public void testLoadSettings() throws Exception {
        Upload f = m_manager.newUpload(UploadSpecification.getSpecificationById("uploadpolycom"));
        f.getSettings();
    }
    
    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Upload f = m_manager.newUpload(UploadSpecification.UNMANAGED);
        f.setName("bezerk");
        m_manager.saveUpload(f);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("upload/SaveUploadExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[upload_id]", f.getPrimaryKey());
        ITable expected = expectedRds.getTable("Upload");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("Upload");

        Assertion.assertEquals(expected, actual);
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/UploadSeed.xml");
        Upload f = m_manager.loadUpload(new Integer(1000));
        assertEquals("test upload", f.getName());
        assertSame(UploadSpecification.UNMANAGED, f.getSpecification());
        
        Integer id = f.getId();        
        m_manager.deleteUpload(f);
        try {
            m_manager.loadUpload(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();        
        assertEquals(0, actual.getTable("Upload").getRowCount());        
    }
    
    public void testGetUpload() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.xml");
        Upload[] f = (Upload[]) m_manager.getUpload().toArray(new Upload[0]);
        assertEquals(2, f.length);
        assertEquals("harriot", f[0].getName());        
        assertEquals("ozzie", f[1].getName());
    }
}
