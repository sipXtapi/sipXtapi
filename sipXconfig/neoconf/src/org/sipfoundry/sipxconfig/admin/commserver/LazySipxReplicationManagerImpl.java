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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.LazyDaemon;

public class LazySipxReplicationManagerImpl implements SipxReplicationManager {
    /**
     * 7s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 7000;

    private Set m_tasks = new HashSet();

    private SipxReplicationManager m_target;

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    public void init() {
        Worker worker = new Worker();
        worker.start();
    }

    public synchronized void generate(DataSet dataSet) {
        m_tasks.add(dataSet);
        notify();
    }

    public synchronized void generateAll() {
        m_tasks.addAll(DataSet.getEnumList());
        notify();
    }

    /* could be private - workaround for checkstyle bug */
    protected synchronized void waitForWork() throws InterruptedException {
        if (m_tasks.isEmpty()) {
            wait();
        }
    }

    private synchronized Set getTasks() {
        if (m_tasks.isEmpty()) {
            return Collections.EMPTY_SET;
        }
        Set oldTasks = m_tasks;
        m_tasks = new HashSet();
        return oldTasks;
    }

    public void setTarget(SipxReplicationManager target) {
        m_target = target;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    /**
     * Worker thread: waits till someone needs to be done, sleeps for a bit and does it.
     */
    private class Worker extends LazyDaemon {
        public Worker() {
            super("Replication worker thread", m_sleepInterval);
        }

        protected void waitForWork() throws InterruptedException {
            LazySipxReplicationManagerImpl.this.waitForWork();
        }

        protected boolean work() {
            Set tasks = getTasks();
            for (Iterator i = tasks.iterator(); i.hasNext();) {
                DataSet ds = (DataSet) i.next();
                m_target.generate(ds);
            }
            return true;
        }
    }
}
