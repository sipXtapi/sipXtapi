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

/**
 * Basic phone setting all phones should implement.  See SettingBeanAdapter for mapping support to 
 * settings
 */
public interface PhoneSettings {

    public static final String DOMAIN_NAME = "domainName";
    
    public static final String OUTBOUND_PROXY = "outboundProxy";
    
    public static final String OUTBOUND_PROXY_PORT = "outboundProxyPort";
    
    public static final String TFTP_SERVER = "tftpServer";
    
    public String getOutboundProxy();
    
    public void setOutboundProxy(String proxy);
    
    public String getOutboundProxyPort();

    public void setOutboundProxyPort(String port);
    
    public String getDomainName();
    
    public void setDomainName(String domainName);
    
    public String getTftpServer();

    public void setTftpServer(String server);
}
