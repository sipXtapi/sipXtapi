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

import junit.framework.TestCase;

import org.apache.commons.collections.BufferOverflowException;

public class JobQueueTest extends TestCase {
    
    private int m_count;
    
    private JobQueue m_queue;
    
    private JobRecord m_stop = new JobRecord();
    
    protected void setUp() {
        m_queue = new JobQueue();        
    }
    
    public void testExceptionOnOverflow() {
        for (int i = 0; i < 5; i++) {
            m_queue.addJob(new JobRecord());
        }
        try {
            m_queue.addJob(new JobRecord());
            fail();
        } catch (BufferOverflowException expected) {
            assertTrue(true);
        }
    }
    
    
    public void testQueue() throws Exception {
        Runnable producer = new Runnable() {
            public void run() {
                for (int i = 0; i < 4; i++) {
                    m_queue.addJob(new JobRecord());
                }
                m_queue.addJob(m_stop);
            }
        };
        
        Runnable consumer = new Runnable() {
            public void run() {
                JobRecord job = null;
                while (job != m_stop) {
                    job = m_queue.removeJob();
                    // artificial delay
                    try {
                        Thread.sleep(100);
                    }
                    catch (InterruptedException ie) {
                        fail();
                    }
                    m_count++;
                }
            }
        };
        
        Thread consumerThread = new Thread(consumer);
        consumerThread.start();
        new Thread(producer).start();
        consumerThread.join();
        assertEquals(5, m_count);
    }
}
