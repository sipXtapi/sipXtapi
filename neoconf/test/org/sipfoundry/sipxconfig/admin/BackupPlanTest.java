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
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.TestUtil;

public class BackupPlanTest extends TestCase {

    private BackupPlan m_backup;

    protected void setUp() throws Exception {
        m_backup = new BackupPlan();
        m_backup.setConfigsScript("mock-backup.sh");
        m_backup.setMailstoreScript("mock-backup.sh");
    }

    public void testBuildExecName() {
        File tmp = new File("/tmp");
        String cmdLine = m_backup.buildExecName(tmp, "kuku");
        assertEquals("/tmp/kuku --non-interactive", cmdLine);
    }

    public void testGetBackupLocations() {
        File[] backupLocations = m_backup.getBackupFiles(new File("."));
        assertEquals(3, backupLocations.length);
        String[] refBackupLocations = {
            "./backup-configs/fs.tar.gz", "./backup-configs/pds.tar.gz",
            "./backup-mailstore/mailstore.tar.gz"
        };
        for (int i = 0; i < refBackupLocations.length; i++) {
            assertEquals(refBackupLocations[i], backupLocations[i].getPath());
        }

    }

    public void testPerform() throws Exception {
        String backupPath = TestHelper.getTestDirectory() + "/backup-" + System.currentTimeMillis();
        File[] backups = m_backup.perform(backupPath, TestUtil.getTestSourceDirectory(this.getClass()));
        assertEquals(3, backups.length);
        assertTrue(backups[0].exists());
        assertTrue(backups[1].exists());
        assertTrue(backups[2].exists());
    }
    
    public void testGetNextBackupDir() {        
        File f = m_backup.getNextBackupDir(new File("."));
        assertTrue(f.getName().matches("\\d{12}"));
    }
    
    public void testLimtedCount() throws Exception {
        m_backup.setLimitedCount(new Integer(2));
        assertNull(m_backup.getOldestPurgableBackup(new String[] { "20050501" }));
        assertEquals("20050501", m_backup.getOldestPurgableBackup(new String[] { "20050501", "20050502"}));        
    }
    
    public void testTimer() throws Exception {        
        BackupPlan plan = new BackupPlan();
        DailyBackupSchedule schedule = new DailyBackupSchedule();
        schedule.setEnabled(true);
        plan.addSchedule(schedule);
        TimerTask task = plan.getTask("root", "bin");
        
        MockControl timerControl = MockClassControl.createStrictControl(Timer.class);
        Timer timer = (Timer) timerControl.getMock();
        Date d = schedule.getTimerDate();        
        timer.schedule(task, d, 1000 * 60 * 60 * 24);
        timerControl.replay();

        plan.schedule(timer, task);
        
        timerControl.verify();
    }
}
