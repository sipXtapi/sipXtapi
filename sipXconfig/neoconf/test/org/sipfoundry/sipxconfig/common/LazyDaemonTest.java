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

import junit.framework.TestCase;

public class LazyDaemonTest extends TestCase {

    private StringBuffer m_testBuffer = new StringBuffer();

    public void testRun() throws Exception {
        LazyDaemonMock mock = new LazyDaemonMock();
        mock.start();
        // do not give it more than 1 second to complete
        mock.join(1000);
        assertEquals("xXxXxX", m_testBuffer.toString());
    }

    private class LazyDaemonMock extends LazyDaemon {
        private int m_i = 0;

        LazyDaemonMock() {
            super(LazyDaemonTest.class.getName(), 2);
        }

        protected void waitForWork() throws InterruptedException {
            m_testBuffer.append("x");
        }

        protected boolean work() {
            m_testBuffer.append("X");
            m_i++;
            return m_i < 3;
        }
    }
}
