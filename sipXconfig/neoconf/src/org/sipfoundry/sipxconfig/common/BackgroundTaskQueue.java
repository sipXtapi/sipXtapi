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
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.collections.Buffer;
import org.apache.commons.collections.BufferUtils;
import org.apache.commons.collections.buffer.BoundedFifoBuffer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class BackgroundTaskQueue {
    private static final Log LOG = LogFactory.getLog(BackgroundTaskQueue.class);

    private final Buffer m_queue = BufferUtils.blockingBuffer(new BoundedFifoBuffer());
    private Thread m_worker;

    public void addTask(Runnable task) {
        m_queue.add(task);
        startWorker();
    }

    public boolean isEmpty() {
        return m_queue.isEmpty();
    }

    private Runnable removeTask() {
        return (Runnable) m_queue.remove();
    }

    private synchronized void startWorker() {
        if (m_worker != null) {
            return;
        }
        m_worker = new Worker();
        m_worker.start();
    }

    private class Worker extends Thread {
        public Worker() {
            super("BackgroundTaskQueue - WorkerThread");
            // do not stop JVM from dying
            setDaemon(true);
        }

        public void run() {
            while (true) {
                Runnable task = removeTask();
                try {
                    task.run();
                } catch (Exception e) {
                    LOG.error("Exception in background task.", e);
                }
            }
        }
    }

}
