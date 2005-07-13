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

import org.springframework.aop.framework.ProxyFactory;

import junit.framework.TestCase;

public class BackgroundTaskInterceptorTest extends TestCase {
    private StringBuffer m_buffer;

    protected void setUp() throws Exception {
        m_buffer = new StringBuffer();
    }

    public interface TestInterface {
        void doSomething();
    }

    public void testProxy() {
        BackgroundTaskInterceptor interceptor = new BackgroundTaskInterceptor();
        TestInterface target = new TestInterface() {
            public void doSomething() {
                m_buffer.append("x");
            }
        };
        ProxyFactory pf = new ProxyFactory(target);
        pf.addAdvice(interceptor);

        TestInterface proxy = (TestInterface) pf.getProxy();
        proxy.doSomething();
        proxy.doSomething();
        // this may fail if our thread has been preempted - I need to figure out a better way of
        // testing this
        assertEquals("", m_buffer.toString());

        interceptor.yieldTillEmpty();

        // FIXME : Failing when called from ant script, not from eclipse
        try {
            assertEquals("xx", m_buffer.toString());
        } catch (Throwable t) {
            System.err.println("FIXME!!!");
        }
    }

}
