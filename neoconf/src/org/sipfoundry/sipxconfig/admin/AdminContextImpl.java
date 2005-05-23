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
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext {

    private String m_binDirectory;

    private String m_backupDirectory;

    public String getBackupDirectory() {
        return m_backupDirectory;
    }

    public void setBackupDirectory(String backupDirectory) {
        m_backupDirectory = backupDirectory;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public BackupPlan getBackupPlan() {
        List plans = getHibernateTemplate().loadAll(BackupPlan.class);
        BackupPlan plan = (BackupPlan) CoreContextImpl
                .requireOneOrZero(plans, "all backup plans");

        // create a new one if one doesn't exists, otherwise
        // risk having 2 or more in database
        if (plan == null) {
            plan = new BackupPlan();
            storeBackupPlan(plan);
        }
        return plan;
    }
    
    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
    }

    public File[] performBackup(BackupPlan plan) {
        return plan.perform(m_backupDirectory, m_binDirectory);
    }
}
