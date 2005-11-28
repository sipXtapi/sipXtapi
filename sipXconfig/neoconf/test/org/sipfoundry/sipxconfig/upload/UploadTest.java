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

import java.io.File;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class UploadTest extends TestCase implements UploadDestination {
    
    private Upload m_upload;
    
    protected void setUp() {
        m_upload = new Upload(UploadSpecification.UNMANAGED);
        m_upload.setModelFilesContext(TestHelper.getModelFilesContext());
        m_upload.setDestination(this);
    }
    
    public void testGetSettingModel() {
        assertNotNull(m_upload.getSettings());
    }
    
    public void testRemove() throws Exception {
        File dir = new File(getUploadDirectory());
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.getSettings().getSetting("files/file1").setValue(file1.getName());
        File file10 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.getSettings().getSetting("files/file10").setValue(file10.getName());
        assertTrue(file1.exists());
        assertTrue(file10.exists());
        m_upload.remove();
        assertFalse(file1.exists());        
        assertFalse(file10.exists());        
    }
    
    /**
     * If file is already deleted, no need to emit error
     */
    public void testRemoveWithFileMissingNoError() throws Exception {
        File dir = new File(getUploadDirectory());
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.getSettings().getSetting("files/file1").setValue(file1.getName());
        file1.delete();
        assertFalse(file1.exists());        
        m_upload.remove();        
    }

    public String getUploadDirectory() {
        return TestHelper.getTestDirectory();
    }
}
