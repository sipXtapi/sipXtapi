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
 * Association between Users and their assigned phones.
 */
public class Line implements Serializable {

    private static final long serialVersionUID = 1L;

    private int m_id = PhoneDao.UNSAVED_ID;

    /** null ok */
    private User m_user;

    private Endpoint m_endpoint;

    private SettingSet m_settings;

    private Credential m_credential;

    public Credential getCredential() {
        return m_credential;
    }

    public void setCredential(Credential credential) {
        m_credential = credential;
    }

    public Endpoint getEndpoint() {
        return m_endpoint;
    }

    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public SettingSet getSettings() {
        return m_settings;
    }

    public void setSettings(SettingSet settings) {
        m_settings = settings;
    }
}
