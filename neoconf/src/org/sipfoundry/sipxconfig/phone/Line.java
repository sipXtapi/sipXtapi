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

import java.io.Serializable;

/**
 * A SIP address represents a line.  Phones have mulitple lines
 * lines can registered on multiple phones.
 */
public class Line implements Serializable {

    private static final long serialVersionUID = 1L;
    
    private int m_id = PhoneDao.UNSAVED_ID;

    private String m_authId;
    
    private String m_extension;
    
    private String m_password;
    
    private String m_userId;
    
    private String m_server;

    /**
     * @return Returns the authId.
     */
    public String getAuthId() {
        return m_authId;
    }
    /**
     * @param authId The authId to set.
     */
    public void setAuthId(String authId) {
        m_authId = authId;
    }
    /**
     * @return Returns the extension.
     */
    public String getExtension() {
        return m_extension;
    }
    /**
     * @param extension The extension to set.
     */
    public void setExtension(String extension) {
        m_extension = extension;
    }
    /**
     * @return Returns the password.
     */
    public String getPassword() {
        return m_password;
    }
    /**
     * @param password The password to set.
     */
    public void setPassword(String password) {
        m_password = password;
    }
    /**
     * @return Returns the server.
     */
    public String getServer() {
        return m_server;
    }
    /**
     * @param server The server to set.
     */
    public void setServer(String server) {
        m_server = server;
    }
    /**
     * @return Returns the userId.
     */
    public String getUserId() {
        return m_userId;
    }
    /**
     * @param userId The userId to set.
     */
    public void setUserId(String userId) {
        m_userId = userId;
    }
    
    public int getId() {
        return m_id;
    }
    
    public void setId(int id) {
        m_id = id;
    }
}
