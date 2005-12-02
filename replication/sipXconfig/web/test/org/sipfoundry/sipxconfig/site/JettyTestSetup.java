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
package org.sipfoundry.sipxconfig.site;

import junit.extensions.TestSetup;
import junit.framework.Test;
import junit.framework.TestCase;

import org.mortbay.jetty.Server;
import org.mortbay.util.InetAddrPort;

/**
 * Assumes you've run ant to build test war file first ant build-test-war
 */
public class JettyTestSetup extends TestSetup {

    private static Server m_server;

    private int m_port = 9999;

    private String m_url = "http://localhost:" + m_port + "/sipxconfig";

    static {
        // shows which URLs were accessed among other diagnotics
        // but slows down testing.
        // System.setProperty("DEBUG", "true");
    }

    public JettyTestSetup(Test test) {
        super(test);
    }

    public String getUrl() {
        return m_url;
    }

    /**
     * "Leaks" the web server on purpose, but does gracefully shutdown server when JVM shutsdown.
     * First test will start server, subsequent tests will use shared server instance.
     */
    protected void setUp() throws Exception {
        if (m_server == null) {
            startServer();
        }
    }

    protected void startServer() throws Exception {
        // uncomment to disable page and component caching
        // System.setProperty("org.apache.tapestry.disable-caching", "true");
        m_server = new Server();
        m_server.addListener(new InetAddrPort(m_port));

        String war = SiteTestHelper.getBuildDirectory() + "/tests/war";
        m_server.addWebApplication("/sipxconfig", war);
        m_server.start();
    }

    /**
     * If you want to run sipXconfig in jetty w/o any tests
     */
    public static void main(String[] args) {
        TestCase notest = new TestCase() {
            // empty
        };
        JettyTestSetup jetty = new JettyTestSetup(notest);
        try {
            jetty.startServer();
            Runtime.getRuntime().addShutdownHook(new Thread(jetty.shutdownHoook()));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    Runnable shutdownHoook() {
        return new Runnable() {
            public void run() {
                try {
                    m_server.stop();
                    m_server = null;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };
    }
}
