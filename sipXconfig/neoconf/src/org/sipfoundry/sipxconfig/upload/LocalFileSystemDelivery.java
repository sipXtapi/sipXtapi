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

/**
 * Copy files to local filesystem into directory.  
 */
public class LocalFileSystemDelivery implements FileDelivery {
    
    private String m_uploadDirectory;
        
    public String getUploadDirectory() {
        return m_uploadDirectory;
    }
    
    public void setUploadDirectory(String uploadDirectory) {
        m_uploadDirectory = uploadDirectory;
    }
}
