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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;

/**
 * Copy files to local filesystem into directory.  
 */
public class LocalFileSystemDelivery implements FileDelivery {
    
    private File m_rootDirectory;
        
    public void deliverFile(File directory, File file) {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = new FileInputStream(new File(directory, file.getPath()));
            out = new FileOutputStream(new File(m_rootDirectory, file.getPath()));
            CopyUtils.copy(in, out);
        } catch (IOException e) {            
            IOUtils.closeQuietly(in);
            IOUtils.closeQuietly(out);
            throw new RuntimeException("Could not deliver file to TFTP directory", e);
        } 
    }
    
    public void removeFile(File directory_, File file) {
        new File(m_rootDirectory, file.getPath()).delete();
    }
    
    public void setRootDirectory(String tftpRoot) {
        m_rootDirectory = new File(tftpRoot);
    }
}
