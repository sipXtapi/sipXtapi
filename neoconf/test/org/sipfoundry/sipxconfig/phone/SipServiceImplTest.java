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

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;


public class SipServiceImplTest extends TestCase {
    
    private SipService m_sip;
    
    private static int s_port = 9023; // not magical, any will do
    
    protected void setUp() {
        m_sip = new SipServiceImpl();
    }    
    
    public void testBranchId() {
        // branch id should make these differ
        assertNotSame(m_sip.getServerVia(), m_sip.getServerVia());
    }
    
    public void testSend() throws Exception {
        ReadSipMessage rdr = new ReadSipMessage();
        
        String msg = "NOTIFY sipuaconfig@localhost.com:" + s_port + " SIP/2.0\r\n" 
            + "Content-Length: 0\r\n" 
            + "\r\n";

        m_sip.send("localhost", s_port, msg);
        
        rdr.shutdown();
        assertEquals(msg, rdr.msg);
    }
    
    public void testRepeatedSend() throws Exception {
        testSend();
        testSend();
        testSend();
        testSend();
        testSend();
    }
    
    static class ReadSipMessage extends Thread {
        
        String msg;
        
        ServerSocket m_server;
        
        ReadSipMessage() throws IOException {
            m_server = new ServerSocket(s_port);
            start();
        }
        
        void shutdown() throws Exception {
            m_server.close();
            join();
        }
        
        public void run() {
            try {
                Socket socket = m_server.accept();
                msg = IOUtils.toString(socket.getInputStream());
            } catch (IOException e) {
                throw new RuntimeException("Failure to read socket", e);
            }
        }
    }

}
