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
public class EndpointLine implements Serializable {

    private static final long serialVersionUID = 1L;
    
    private String m_label;
    
    private int m_id = PhoneDao.UNSAVED_ID;
    
    private Line m_line;
    
    private Endpoint m_endpoint;
    
    private SettingSet m_settings;

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

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public Line getLine() {
        return m_line;
    }

    public void setLine(Line line) {
        m_line = line;
    }

    public SettingSet getSettings() {
        return m_settings;
    }

    public void setSettings(SettingSet settings) {
        m_settings = settings;
    }
}
