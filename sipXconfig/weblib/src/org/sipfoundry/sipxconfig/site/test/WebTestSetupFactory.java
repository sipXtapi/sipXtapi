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
package org.sipfoundry.sipxconfig.site.test;

import junit.extensions.TestSetup;
import junit.framework.Test;

import org.mortbay.jetty.Server;
import org.mortbay.util.InetAddrPort;

/**
 * Assumes you've run ant to build test war file first ant build-test-war
 */
public class WebTestSetupFactory {

    private Server m_server;

    private int m_port = 9999;

    private String m_webAppName;
    
    private String m_buildDirectory;

    static {
        // shows which URLs were accessed among other diagnotics
        // but slows down testing.
        // System.setProperty("DEBUG", "true");
    }
    
    public TestSetup createTestSetup(Test test) {
        return new JettySetup(test);
    }
    
    class JettySetup extends TestSetup {
        
        JettySetup(Test test) {
            super(test);
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
    }   

    public String getBuildDirectory() {
        return m_buildDirectory;
    }

    public void setBuildDirectory(String buildDirectory) {
        m_buildDirectory = buildDirectory;
    }


    public int getPort() {
        return m_port;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public String getWebAppName() {
        return m_webAppName;
    }

    public void setWebAppName(String webAppName) {
        m_webAppName = webAppName;
    }

    public String getUrl() {
        return "http://localhost:" + m_port + "/" + m_webAppName;
    }

    protected void startServer() throws Exception {
        // uncomment to disable page and component caching
        // System.setProperty("org.apache.tapestry.disable-caching", "true");
        m_server = new Server();
        m_server.addListener(new InetAddrPort(m_port));

        // add the sipXconfig web application
        String war = m_buildDirectory + "/tests/war";
        m_server.addWebApplication("/" + m_webAppName, war);
        
        m_server.start();
    }
    
    private void shutdownJetty() {
        if (m_server != null) {
            try {
                m_server.stop();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            m_server = null;
        }        
    }
        
    private void startJetty() {
        try {
            startServer();
            Runtime.getRuntime().addShutdownHook(new Thread(shutdownHoook()));
        } catch (Exception e) {
            e.printStackTrace();
        }        
    }

    /**
     * If you want to run sipXconfig in jetty w/o any tests
     */
    protected void runMain(String[] args) {
        if (args.length > 0 && "shutdown".equals(args[0])) {
            shutdownJetty();           
        } else {
            startJetty();
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
