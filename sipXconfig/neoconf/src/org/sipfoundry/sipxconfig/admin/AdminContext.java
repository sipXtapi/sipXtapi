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
     * If your service requires a patch be run AND your service needs to be
     * executed BEFORE admin user logs in, you may call this.  For the most
     * part, most of the system will not be excersised until all patches have
     * been successfully applied.
     *  
     * @throws PatchNotAppliedException
     */
    public void requirePatch(Integer patchId);
}
