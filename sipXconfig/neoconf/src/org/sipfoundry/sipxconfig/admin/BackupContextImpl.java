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
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Backup provides Java interface to backup scripts
 */
public class BackupContextImpl implements BackupContext {
    private static final Log LOG = LogFactory.getLog(BackupContextImpl.class);

    private static final String MAILSTORE = "mailstore.tar.gz";
    private static final String CONFIGS = "fs.tar.gz";
    private static final String DATABASE = "pds.tar.gz";

    private static final String BACKUP_CONFIGS = "backup-configs";
    private static final String BACKUP_MAILSTORE = "backup-mailstore";

    private static final String SCRIPT_SUFFIX = ".sh";
    private static final String OPTIONS = "--non-interactive";

    private String m_binDirectory;
    private boolean m_voicemail = true;
    private boolean m_database = true;
    private boolean m_configs = true;

    public BackupContextImpl(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String[] perform(String backupPath) {
        String errorMsg = "Errors when creating backup.";
        try {
            File backupDir = new File(backupPath);
            if (!backupDir.isDirectory()) {
                backupDir.mkdirs();
            }
            perform(backupDir);
            return getBackupLocations();
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

    private int perform(File workingDir) throws IOException, InterruptedException {
        List processes = new ArrayList();
        File binDir = new File(m_binDirectory);
        if (m_configs || m_database) {
            String cmdLine = buildExecName(binDir, BACKUP_CONFIGS + SCRIPT_SUFFIX);
            processes.add(exec(cmdLine, workingDir));

        }
        if (m_voicemail) {
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

    String[] getBackupLocations() {
        List paths = new ArrayList();
        if (m_configs) {
            File path = new File(BACKUP_CONFIGS);
            File configs = new File(path, CONFIGS);
            paths.add(configs.getPath());
        }
        if (m_database) {
            File path = new File(BACKUP_CONFIGS);
            File database = new File(path, DATABASE);
            paths.add(database.getPath());
        }
        if (m_voicemail) {
            File path = new File(BACKUP_MAILSTORE);
            File mailstore = new File(path, MAILSTORE);
            paths.add(mailstore.getPath());
        }
        return (String[]) paths.toArray(new String[paths.size()]);
    }

    public boolean getConfigs() {
        return m_configs;
    }

    public void setConfigs(boolean configs) {
        m_configs = configs;
    }

    public boolean getDatabase() {
        return m_database;
    }

    public void setDatabase(boolean database) {
        m_database = database;
    }

    public boolean getVoicemail() {
        return m_voicemail;
    }

    public void setVoicemail(boolean voicemail) {
        m_voicemail = voicemail;
    }
}
