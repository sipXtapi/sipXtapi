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
package org.sipfoundry.sipxconfig.admin;

import java.io.File;

import junit.framework.TestCase;

public class BackupPlanTest extends TestCase {

    //private static final String BIN_DIR = "/opt/work-2.8/sipx/bin";
    private BackupPlan m_backup;

    protected void setUp() throws Exception {
        m_backup = new BackupPlan();
    }

    public void testBuildExecName() {
        File tmp = new File("/tmp");
        String cmdLine = m_backup.buildExecName(tmp, "kuku");
        assertEquals("/tmp/kuku --non-interactive", cmdLine);
    }

    public void testGetBackupLocations() {
        File[] backupLocations = m_backup.getBackupFiles();
        assertEquals(3, backupLocations.length);
        String[] refBackupLocations = {
            "backup-configs/fs.tar.gz", "backup-configs/pds.tar.gz",
            "backup-mailstore/mailstore.tar.gz"
        };
        for (int i = 0; i < refBackupLocations.length; i++) {
            assertEquals(refBackupLocations[i], backupLocations[i].getPath());
        }

    }

    /*
     * public void testPerform() throws Exception { String tempDir = SystemUtils.JAVA_IO_TMPDIR;
     * Process process = m_backup.exec( "/opt/work-2.8/sipx/bin/backup-configs.sh
     * --non-interactive", new File(tempDir)); assertEquals(0, process.waitFor()); }
     */
}
