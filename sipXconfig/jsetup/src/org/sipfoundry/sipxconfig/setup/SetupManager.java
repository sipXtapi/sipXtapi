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
package org.sipfoundry.sipxconfig.setup;

import java.io.File;
import java.io.FilenameFilter;

public class SetupManager {
    
    private String m_backupDirectory;
    
    public String getBackupDirectory() {
        return m_backupDirectory;
    }

    public void setBackupDirectory(String backupDirectory) {
        m_backupDirectory = backupDirectory;
    }

    public File[] getBackups() {
        File f = new File(getBackupDirectory());
        File[] backups = f.listFiles(new BackupFileFilter());
        return backups;
    }
    
    static class BackupFileFilter implements FilenameFilter {
        public boolean accept(File dir, String name) {
            return name.endsWith(".tgz");
        }        
    }
}
