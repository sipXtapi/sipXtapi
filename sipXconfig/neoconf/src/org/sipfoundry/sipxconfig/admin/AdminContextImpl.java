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
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.List;
import java.util.Timer;

import net.sf.hibernate.HibernateException;
import net.sf.hibernate.Session;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.jdbc.support.JdbcUtils;
import org.springframework.orm.hibernate.HibernateCallback;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext, ApplicationListener {

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
    
    public void setPatchApplied(String patch) {
        getHibernateTemplate().execute(new ApplyPatch(patch));
    }

    class ApplyPatch implements HibernateCallback {
        
        private String m_patch;
        
        ApplyPatch(String patch) {
            m_patch = patch;
        }
        
        public Object doInHibernate(Session session) throws HibernateException, SQLException {
            Connection connection = session.connection();
            Statement statement = connection.createStatement();
            try {
                statement.executeUpdate("insert into patch (name) values ('" + m_patch + "')");
            } finally {
                JdbcUtils.closeStatement(statement);
            }
            return null;
        }        
    }

}
