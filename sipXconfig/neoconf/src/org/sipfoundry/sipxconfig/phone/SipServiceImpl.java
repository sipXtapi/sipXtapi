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
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


/**
 * Implementation note: This is very, very basic sip implementation. Going forward, either
 * use sipXtack java binding or interact w/sipXpublisher thru some RPC to have it deliver
 * sip messages on behalf or sipXconfig
 */
public class SipServiceImpl implements SipService {
    
    private static final long RANDOM_MAX = Long.MAX_VALUE;
    
    private static final int DEFAULT_SIP_PORT = 5060;

    private static Log s_log = LogFactory.getLog(SipServiceImpl.class);

    /** Example:  Tue, 15 Nov 1994 08:12:31 GMT */
    private SimpleDateFormat m_dateFormat;

    private String m_serverName = "localhost";

    private int m_serverPort = DEFAULT_SIP_PORT;
    
    private String m_proxyHost = m_serverName;
    
    private int m_proxyPort = DEFAULT_SIP_PORT;
    
    public SipServiceImpl() {
        m_dateFormat = new SimpleDateFormat("EEE, d MMM yyyy kk:mm:ss z");
        m_dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        
        try {
            m_serverName = InetAddress.getLocalHost().getHostAddress();
        } catch (UnknownHostException nonFatal) {
            // For now, we never read response and could care less if return
            // address is unattainable.  Next iteration of this SipService
            // may use a real stack w/access to config.defs.
            nonFatal.printStackTrace();
        }
    }

    public void setProxyHost(String proxy) {
        m_proxyHost = proxy;
    }

    public void setProxyPort(int port) {
        m_proxyPort = port;
    }

    public String getServerVia() {
        return "SIP/2.0/UDP " + getFromServer() + ";branch="
                + generateBranchId();
    }

    public String generateBranchId() {
        return generateUniqueId();
    }

    public String getFromServerName() {
        return m_serverName;
    }

    public void setFromServerName(String serverName) {
        m_serverName = serverName;
    }

    public int getFromServerPort() {
        return m_serverPort;
    }

    public void setFromServerPort(int serverPort) {
        m_serverPort = serverPort;
    }
    
    private String getFromServer() {
        String from;
        if (getFromServerPort() == DEFAULT_SIP_PORT) {
            from  = getFromServerName();
        } else {
            from  = getFromServerName() + ":" + getFromServerPort();
        }
        
        return from;
    }

    public String getServerUri() {        
        return "sip:sipuaconfig@" + getFromServer();
    }

    public void send(String sipMsg) throws IOException {
        // not particular reason it's UDP other than we do not
        // expect a response so this seems more appropriate.
        s_log.info(sipMsg);
        DatagramSocket socket = new DatagramSocket();
        byte[] sipBytes = sipMsg.getBytes();
        InetAddress toAddress = InetAddress.getByName(m_proxyHost);
        DatagramPacket packet = new DatagramPacket(sipBytes, 0, sipBytes.length, 
                toAddress, m_proxyPort);
        socket.send(packet);
        socket.disconnect();
    }
    
    public String getCurrentDate() {
        return getDateFormat().format(new Date());
    }

    public DateFormat getDateFormat() {
        return m_dateFormat;
    }

    public String generateCallId() {
        return "90d3f2-" + generateUniqueId();
    }

    String generateUniqueId() {
        long l = (long) (Math.random() * RANDOM_MAX);
        return Long.toHexString(l);
    }
}
