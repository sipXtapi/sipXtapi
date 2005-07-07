/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import org.apache.commons.collections.Buffer;
import org.apache.commons.collections.BufferUtils;
import org.apache.commons.collections.buffer.BoundedFifoBuffer;

/**
 * Bounded blocking queue. This is used as alternative to JMS queue because having the queue be
 * remote is not nec., however if this changes, JobRecord is Serializable so this can be converted
 * back to a JMS queue rather easily.
 */
public class JobQueue {

    /** 5 seemed like a reasonable number, no magic to this number */
    private static final int QUEUE_SIZE = 5;

    private Buffer m_queue;

    public JobQueue() {
        m_queue = BufferUtils.blockingBuffer(new BoundedFifoBuffer(QUEUE_SIZE));
    }

    public void addJob(JobRecord job) {
        m_queue.add(job);
    }

    public JobRecord removeJob() {
        return (JobRecord) m_queue.remove();
    }

    public boolean isEmpty() {
        return m_queue.isEmpty();
    }
}
