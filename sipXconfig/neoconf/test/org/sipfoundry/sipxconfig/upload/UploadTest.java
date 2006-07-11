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

public class UploadTest extends TestCase {
    
    private Upload m_upload;
    
    protected void setUp() {
        m_upload = new Upload(UploadSpecification.UNMANAGED);
        m_upload.setDirectoryId(String.valueOf(System.currentTimeMillis()));
        m_upload.setModelFilesContext(TestHelper.getModelFilesContext());
        m_upload.setUploadRootDirectory(TestHelper.getTestDirectory() + "/upload");
        m_upload.setDestinationDirectory(mkdirs(TestHelper.getTestDirectory() + "/tftproot"));
        m_upload.initialize();
    }
    
    private String mkdirs(String dir) {
        new File(dir).mkdirs();
        return dir;
    }
    
    public void testGetSettingModel() {
        assertNotNull(m_upload.getSettings());
    }
    
    public void testRemove() throws Exception {
        File dir = new File(mkdirs(m_upload.getUploadDirectory()));
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.setSettingValue("files/file1", file1.getName());
        File file10 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.setSettingValue("files/file10", file10.getName());
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
        File dir = new File(mkdirs(m_upload.getUploadDirectory()));
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.setSettingValue("files/file1", file1.getName());
        file1.delete();
        assertFalse(file1.exists());        
        m_upload.remove();        
    }
    
    public void testDeploy() throws Exception {
        File dir = new File(mkdirs(m_upload.getUploadDirectory()));
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.setSettingValue("files/file1", file1.getName());
        m_upload.deploy();
        assertTrue(m_upload.isDeployed());
        assertTrue(new File(m_upload.getDestinationDirectory() + "/" + file1.getName()).exists());        
    }
    
    public void testUndeploy() throws Exception {
        File dir = new File(mkdirs(m_upload.getDestinationDirectory()));
        File file1 = File.createTempFile("upload-test", ".dat", dir);
        m_upload.setSettingValue("files/file1", file1.getName());
        m_upload.undeploy();
        assertFalse(m_upload.isDeployed());
        assertFalse(file1.exists());                
    }
}
