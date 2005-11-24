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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.upload.Firmware;
import org.sipfoundry.sipxconfig.upload.FirmwareManufacturer;

public class FirmwareTest extends TestCase {
    
    public void testGetSettingModel() {
        Firmware f = new Firmware();
        f.setModelFilesContext(TestHelper.getModelFilesContext());
        f.setManufacturer(FirmwareManufacturer.UNMANAGED);
        assertNotNull(f.getSettings());
    }
    
    // work in progress
//    public void testGetFiles() throws Exception {
//        Firmware f = new Firmware();
//        Integer uid = new Integer(1);
//        f.setUniqueUploadId(uid);
//        File d = createTempDir(new File(TestHelper.getTestDirectory()), uid.toString());
//        File f0 = createTempFile(d);
//        File f1 = createTempFile(d);
//        File[] files = f.getFiles();
//        assertNotNull(files);
//        assertEquals(2, files.length);
//        assertEquals(f0.getPath(), files[0].getPath());
//        assertEquals(f1.getPath(), files[1].getPath());
//    }
//    
//    private File createTempDir(File dir, String subdir) throws IOException {        
//        File parent = new File(dir, "upload" + Long.toString(System.currentTimeMillis()));
//        File temp = new File(parent, subdir);
//        temp.mkdirs();
//        return temp;        
//    }
//    
//    private File createTempFile(File dir) throws IOException {
//        File f = File.createTempFile("upload", null, dir);
//        return f;                
//    }
}
