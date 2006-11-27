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
import java.io.IOException;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
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
    
    public void testDeploy() {        
        ZipUpload.deployZipFile(m_expandDir, m_zipFile);
    }
    
    public void testUndeply() {
        testDeploy();
        assertTrue(new File(m_expandDir, "zip-test/subdir/file1.txt").exists());
        ZipUpload.undeployZipFile(m_expandDir, m_zipFile);        
        assertFalse(new File(m_expandDir, "zip-test/subdir/file1.txt").exists());
    }

}
