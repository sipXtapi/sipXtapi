/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.LazyDaemon;

public class LazyProfileManagerImpl implements ProfileManager {
    private Set m_phoneIds = new HashSet();

    private ProfileManager m_target;

    private int m_sleepInterval;

    private Worker m_worker;

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    public void setTarget(ProfileManager target) {
        m_target = target;
    }

    public synchronized void generateProfilesAndRestart(Collection phoneIds) {
        m_phoneIds.addAll(phoneIds);
        m_worker.workScheduled();
        notify();
    }

    public synchronized void generateProfileAndRestart(Integer phoneId) {
        m_phoneIds.add(phoneId);
        m_worker.workScheduled();
        notify();
    }

    private synchronized void waitForWork() throws InterruptedException {
        if (m_phoneIds.isEmpty()) {
            wait();
        }
    }

    public void init() {
        m_worker = new Worker();
        m_worker.start();
    }

    private synchronized Set getTasks() {
        if (m_phoneIds.isEmpty()) {
            return Collections.EMPTY_SET;
        }
        Set oldTasks = m_phoneIds;
        m_phoneIds = new HashSet();
        return oldTasks;
    }

    private class Worker extends LazyDaemon {
        public Worker() {
            super("Profile Manager thread", m_sleepInterval);
        }

        protected void waitForWork() throws InterruptedException {
            LazyProfileManagerImpl.this.waitForWork();
        }

        protected boolean work() {
            m_target.generateProfilesAndRestart(getTasks());
            return true;
        }
    }
}
