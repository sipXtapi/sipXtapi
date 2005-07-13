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

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;

public class BackgroundTaskInterceptor implements MethodInterceptor {
    private BackgroundTaskQueue m_queue = new BackgroundTaskQueue();

    public Object invoke(MethodInvocation invocation) throws Throwable {
        InvocationTask task = new InvocationTask(invocation);
        m_queue.addTask(task);
        return null;
    }

    private static class InvocationTask implements Runnable {
        private final MethodInvocation m_invocation;

        public InvocationTask(MethodInvocation invocation) {
            m_invocation = invocation;
        }

        public void run() {
            try {
                m_invocation.proceed();
            } catch (Throwable e) {
                if (e instanceof RuntimeException) {
                    throw (RuntimeException) e;
                }
                throw new RuntimeException(e);
            }
        }
    }

    /**
     * Use only for testing. There is no guarantee if it ever returns.
     */
    void yieldTillEmpty() {
        while (!m_queue.isEmpty()) {
            Thread.yield();
        }
    }
}
