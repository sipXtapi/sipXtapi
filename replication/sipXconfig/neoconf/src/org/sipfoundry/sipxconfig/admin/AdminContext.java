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

public interface AdminContext {
    
    public static final String CONTEXT_BEAN_NAME = "adminContext";

    public abstract BackupPlan getBackupPlan();
    
    public abstract void storeBackupPlan(BackupPlan plan);
    
    public File[] performBackup(BackupPlan plan);
    
    /** 
     * After successfully sending event to application to perform a database
     * related task, remove task from initialization task table.
     */
    public void deleteInitializationTask(String task);
    
    public String[] getInitializationTasks();
}
