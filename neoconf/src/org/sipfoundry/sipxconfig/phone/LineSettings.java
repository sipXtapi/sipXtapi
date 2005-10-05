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
public interface LineSettings {
    
    public static final String REGISTRATION_SERVER = "registrationServer";
    
    public static final String REGISTRATION_SERVER_PORT = "registrationServerPort";
    
    public static final String DISPLAY_NAME = "displayName";
    
    public static final String USER_ID = "userId";
    
    public static final String AUTHORIZATION_ID = "authorizationId";
    
    public static final String PASSWORD = "password";
    
    public static final String DOMAIN_NAME = "domainName";
    
    public static final String AUTHORIZATION_REALM = "authorizationRealm";
    
    public static final String VOICEMAIL = "voiceMail";

    public String getRegistrationServer();
    
    public void setRegistrationServer(String server);
    
    public String getRegistrationServerPort();
    
    public void setRegistrationServerPort(String port);
    
    public String getDisplayName();
    
    public void setDisplayName(String displayName);
        
    public String getUserId();
    
    public void setUserId(String userId);
    
    public String getAuthorizationRealm();
    
    public void setAuthorizationRealm(String realm);
    
    public String getAuthorizationId();
    
    public void setAuthorizationId(String authId);
    
    public String getPassword();
    
    public void setPassword(String password);
    
    public String getDomainName();
    
    public void setDomainName(String address);
    
    public String getVoiceMail();
    
    public void setVoiceMail(String voiceMail);
}
