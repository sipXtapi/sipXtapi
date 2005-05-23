/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class BackupPlan extends BeanWithId {
    
    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    private static final String MAILSTORE = "mailstore.tar.gz";
    private static final String CONFIGS = "fs.tar.gz";
    private static final String DATABASE = "pds.tar.gz";

    private static final String BACKUP_CONFIGS = "backup-configs";
    private static final String BACKUP_MAILSTORE = "backup-mailstore";

    private static final String SCRIPT_SUFFIX = ".sh";
    private static final String OPTIONS = "--non-interactive";

    private boolean m_voicemail = true;
    private boolean m_database = true;
    private boolean m_configs = true;    
    private Integer m_limitedCount;
    
    private Collection m_schedules = new ArrayList(0);
    
    public File[] perform(String backupPath, String binPath) {
        String errorMsg = "Errors when creating backup.";
        try {
            File backupDir = new File(backupPath);
            if (!backupDir.isDirectory()) {
                backupDir.mkdirs();
            }
            File binDir = new File(binPath);
            perform(backupDir, binDir);
            return getBackupFiles();
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
        return null;
    }

    String buildExecName(File path, String script) {
        File scriptPath = new File(path, script);
        StringBuffer cmdLine = new StringBuffer(scriptPath.getAbsolutePath());
        cmdLine.append(' ');
        cmdLine.append(OPTIONS);
        return cmdLine.toString();
    }

    private Process exec(String cmdLine, File workingDir) throws IOException {
        Runtime runtime = Runtime.getRuntime();
        return runtime.exec(cmdLine, ArrayUtils.EMPTY_STRING_ARRAY, workingDir);
    }

    private int perform(File workingDir, File binDir) throws IOException, InterruptedException {
        List processes = new ArrayList();
        if (isConfigs() || isDatabase()) {
            String cmdLine = buildExecName(binDir, BACKUP_CONFIGS + SCRIPT_SUFFIX);
            processes.add(exec(cmdLine, workingDir));

        }
        if (isVoicemail()) {
            String cmdLine = buildExecName(binDir, BACKUP_MAILSTORE + SCRIPT_SUFFIX);
            processes.add(exec(cmdLine, workingDir));

        }
        int exitCode = 0;
        for (Iterator i = processes.iterator(); i.hasNext();) {
            Process proc = (Process) i.next();
            int code = proc.waitFor();
            if (code != 0) {
                exitCode = code;
            }
        }
        return exitCode;
    }

    File[] getBackupFiles() {
        List files = new ArrayList();
        if (isConfigs()) {
            File path = new File(BACKUP_CONFIGS);
            File configs = new File(path, CONFIGS);
            files.add(configs);
        }
        if (isDatabase()) {
            File path = new File(BACKUP_CONFIGS);
            File database = new File(path, DATABASE);
            files.add(database);
        }
        if (isVoicemail()) {
            File path = new File(BACKUP_MAILSTORE);
            File mailstore = new File(path, MAILSTORE);
            files.add(mailstore);
        }
        return (File[]) files.toArray(new File[files.size()]);
    }
    
    public void addSchedule(DailyBackupSchedule dailySchedule) {
        m_schedules.add(dailySchedule);
        dailySchedule.setBackupPlan(this);
    }
    
    public Collection getSchedules() {
        return m_schedules;
    }
    
    public void setSchedules(Collection schedules) {
        m_schedules = schedules;
    }

    public Integer getLimitedCount() {
        return m_limitedCount;
    }

    public void setLimitedCount(Integer limitedCount) {
        m_limitedCount = limitedCount;
    }

    public boolean isConfigs() {
        return m_configs;
    }
    
    public void setConfigs(boolean configs) {
        m_configs = configs;
    }
    
    public boolean isDatabase() {
        return m_database;
    }
    
    public void setDatabase(boolean database) {
        m_database = database;
    }
    
    public boolean isVoicemail() {
        return m_voicemail;
    }
    
    public void setVoicemail(boolean voicemail) {
        m_voicemail = voicemail;
    }
}
