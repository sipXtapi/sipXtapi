/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Timer;
import java.util.TimerTask;

import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class LdapImportTrigger implements ApplicationListener {
    private LdapManager m_ldapManager;

    private LdapImportManager m_ldapImportManager;

    private Timer m_timer;

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setLdapImportManager(LdapImportManager ldapImportManager) {
        m_ldapImportManager = ldapImportManager;
    }

    /**
     * start timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent) {
            CronSchedule schedule = m_ldapManager.getConnectionParams().getSchedule();
            onScheduleChanged(schedule);
        } else if (event instanceof ScheduleChangedEvent) {
            ScheduleChangedEvent sce = (ScheduleChangedEvent) event;
            onScheduleChanged(sce.getSchedule());
        }
    }

    private synchronized void onScheduleChanged(CronSchedule schedule) {
        if (m_timer != null) {
            m_timer.cancel();
        }
        TimerTask ldapImportTask = new LdapImportTask(m_ldapImportManager);
        m_timer = schedule.schedule(ldapImportTask);
    }

    public static final class ScheduleChangedEvent extends ApplicationEvent {
        private CronSchedule m_schedule;

        public ScheduleChangedEvent(CronSchedule schedule, Object eventSource) {
            super(eventSource);
            m_schedule = schedule;
        }

        public CronSchedule getSchedule() {
            return m_schedule;
        }
    }

    private static final class LdapImportTask extends TimerTask {
        private LdapImportManager m_ldapImportManager;

        public LdapImportTask(LdapImportManager ldapImportManager) {
            m_ldapImportManager = ldapImportManager;

        }

        public void run() {
            m_ldapImportManager.insert();
        }
    }

}