/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ZipUploadTest extends TestCase {    
    private File m_expandDir;
    private File m_zipFile;
    
    protected void setUp() throws IOException {
        File thisDir = new File(TestUtil.getTestSourceDirectory(getClass()));
        m_zipFile = new File(thisDir, "zip-test.zip");
        m_expandDir = new File(new File(TestHelper.getTestDirectory()), "zip-test");
        FileUtils.deleteDirectory(m_expandDir);
        m_expandDir.mkdirs();        
    }    
    
    public void testDeployUndeploy() throws Exception {
        ZipUpload.deployZipFile(m_expandDir, m_zipFile);
        File file1 = new File(m_expandDir, "zip-test/subdir/file1.txt");
        assertTrue(file1.exists());
        assertTrue(IOUtils.contentEquals(getClass().getResourceAsStream("file1.txt"), 
                new FileInputStream(file1)));
        File file3 = new File(m_expandDir, "zip-test/file3.bin");
        assertTrue(file3.exists());
        InputStream stream3 = getClass().getResourceAsStream("file3.bin");
        assertTrue(IOUtils.contentEquals(stream3, new FileInputStream(file3)));
        ZipUpload.undeployZipFile(m_expandDir, m_zipFile);        
        assertFalse(file1.exists());
    }
    
    public void testFile() throws Exception {
        File file = File.createTempFile("file3", "bin");
        FileOutputStream stream = new FileOutputStream(file);
        for(int i = 100; i < 400; i++) {
            stream.write(i);
        }
        stream.close();        
    }
}