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
 * Assumes you've run ant to build test war file first
 *   ant build-test-war
 */
public class JettyTestSetup extends TestSetup {

    private Server m_server;
    
    private int m_port = 9999;
    
    private String m_url = "http://localhost:" + m_port + "/sipxconfig";
    
    static {
        // shows which URLs were accessed among other diagnotics
        // but slows down testing.
        //System.setProperty("DEBUG", "true");
    }

    public JettyTestSetup(Test test) {
        super(test);
    }

    public String getUrl() {
        return m_url;
    }
    
    protected void setUp() throws Exception {        
        m_server = new Server();
        m_server.addListener(new InetAddrPort(m_port));
        
        String war = SiteTestHelper.getBuildDirectory() + "/tests/war";
        m_server.addWebApplication("/sipxconfig", war);
        m_server.start();
    }

    protected void tearDown() throws Exception {
        m_server.stop();
        m_server = null;
    }
    
    /**
     * If you want to run sipXconfig in jetty w/o any tests
     */
    public static void main(String[] args) {
        Test empty = new TestCase() {};
        JettyTestSetup jetty = new JettyTestSetup(empty);
        try {
            jetty.setUp();
            // set breakpoint on teardown and go to http://localhost:9999/sipxconfig
            jetty.tearDown();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}