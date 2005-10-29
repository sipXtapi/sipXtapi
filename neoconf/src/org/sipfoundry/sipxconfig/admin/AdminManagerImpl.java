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
import java.util.Collection;
import java.util.List;
import java.util.Timer;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminManagerImpl extends HibernateDaoSupport implements AdminManager, ApplicationListener {

    private String m_binDirectory;

    private String m_backupDirectory;
    
    private Timer m_timer;
    
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
        BackupPlan plan = (BackupPlan) DaoUtils.requireOneOrZero(plans, "all backup plans");

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
        resetTimer(plan);
    }
    
    public File[] performBackup(BackupPlan plan) {
        return plan.perform(m_backupDirectory, m_binDirectory);
    }

    /**
     * start backup timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener interface are 
        // automatically registered
        if (event instanceof ApplicationInitializedEvent) {
            resetTimer(getBackupPlan());
        }
    }

    private void resetTimer(BackupPlan plan) {
        if (m_timer != null) {
            m_timer.cancel();
        }
        m_timer = new Timer(false); // deamon, dies with main thread
        plan.schedule(m_timer, m_backupDirectory, m_binDirectory); 
    }
    
    
    public String[] getInitializationTasks() {
        List l = getHibernateTemplate().findByNamedQuery("taskNames");
        return (String[]) l.toArray(new String[l.size()]);
    }
    
    public void deleteInitializationTask(String task) {
        Collection c = getHibernateTemplate().find("from " + InitializationTask.class.getName() 
                + " t where t.task = '" + task + "'");
        getHibernateTemplate().deleteAll(c);
    }
}

