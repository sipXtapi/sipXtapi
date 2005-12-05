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

import java.io.InputStream;

import junit.framework.TestCase;

public class SipxProcessContextImplTest extends TestCase {
    SipxProcessContextImpl m_processContextImpl;

    protected void setUp() throws Exception {
        m_processContextImpl = new SipxProcessContextImpl() {
            protected InputStream getStatusStream() {
                return SipxProcessContextImplTest.class.getResourceAsStream("status.test.xml");
            }

            protected InputStream getTopologyAsStream() {
                return SipxProcessContextImplTest.class.getResourceAsStream("topology.test.xml");
            }
        };
    }

    public void testConstructRestartUrl() {
        String[] url = m_processContextImpl.constructCommandUrls(
                SipxProcessContext.Process.REGISTRAR.getName(),
                SipxProcessContext.Command.RESTART);

        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=SIPRegistrar",
                url[0]);
    }

    public void testRetrieveProcessUrls() throws Exception {
        String[] urls = m_processContextImpl.getProcessMonitorUrls();
        assertEquals(2, urls.length);
        assertEquals("https://localhost:8091/cgi-bin/processmonitor/process.cgi", urls[0]);
        assertEquals("https://192.168.0.27:8091/cgi-bin/processmonitor/process.cgi", urls[1]);
    }

    public void testConstructStatusUrls() throws Exception {
        final String FIRST_URL = "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=status";

        String[] urls = m_processContextImpl.constructStatusUrls();
        assertEquals(1, urls.length);
        assertEquals(FIRST_URL, urls[0]);
    }

    public void testConstructCommandUrls() throws Exception {
        final String SERVICE_NAME = "SomeService";
        final SipxProcessContext.Command COMMAND = SipxProcessContext.Command.START;
        final String FIRST_URL = "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command="
                + COMMAND.getName() + "&process=" + SERVICE_NAME;

        String[] urls = m_processContextImpl.constructCommandUrls(SERVICE_NAME, COMMAND);
        assertEquals(2, urls.length);
        assertEquals(FIRST_URL, urls[0]);
    }

    // Test getting status. Work with persisted XML output, rather than making a live call
    // to the server, to avoid requiring a server to be running. We override the method
    // getStatusStream of SipxProcessContextImpl for this purpose.
    public void testGetStatus() throws Exception {
        ServiceStatus[] status = m_processContextImpl.getStatus();
        assertEquals(8, status.length);
        assertEquals("ConfigServer", status[0].getServiceName());
    }
}
