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
 * Information that authorizes users and lines
 */
public class Credential {

    private int m_id = PhoneContext.UNSAVED_ID;

    private String m_authId;

    private String m_password;

    private String m_realm;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getAuthId() {
        return m_authId;
    }

    public void setAuthId(String authId) {
        m_authId = authId;
    }

    public String getRealm() {
        return m_realm;
    }

    public void setRealm(String realm) {
        m_realm = realm;
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }
}
