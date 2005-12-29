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

import java.util.Arrays;

import junit.framework.TestCase;

public class BackgroundTaskQueueTest extends TestCase {
    public void testQueue() throws Exception {
        BackgroundTaskQueue queue = new BackgroundTaskQueue();
        final Thread testThread = Thread.currentThread();
        queue.suspend();

        final int taskNum = 97;
        final StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < taskNum; i++) {
            Runnable task = new Runnable() {
                public void run() {
                    buffer.append("A");
                    assertNotSame(testThread, Thread.currentThread());
                }
            };
            queue.addTask(task);
        }

        assertEquals("", buffer.toString());

        queue.resume();
        
        char[] expected = new char[taskNum];
        Arrays.fill(expected, 'A');
        
        queue.yieldTillEmpty();
        
        assertEquals(new String(expected), buffer.toString());
    }

    public void testQueueException() throws Exception {
        BackgroundTaskQueue queue = new BackgroundTaskQueue();

        Runnable taskWithException = new Runnable() {
            public void run() {
                throw new RuntimeException("Ignore this exception - part of testing");
            }
        };
        queue.addTask(taskWithException);
        final StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < 3; i++) {
            Runnable task = new Runnable() {
                public void run() {
                    buffer.append("A");
                }
            };
            queue.addTask(task);
        }

        // give it a chance to do something
        queue.yieldTillEmpty();

        assertEquals("AAA", buffer.toString());
    }

}
