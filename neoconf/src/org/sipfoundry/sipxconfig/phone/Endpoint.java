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
 * Database object representing an actualy physical phone you can touch.
 */
public class Endpoint implements Serializable {

    private static final long serialVersionUID = 1L;

    private int m_id = -1;

    private String m_description;

    private String m_serialNumber;

    private String m_phoneId;
    
    private SettingSet m_settings;

    /**
     * @return ids used in PhoneFactory
     */
    public String getPhoneId() {
        return m_phoneId;
    }

    /**
     * @param phoneId used in PhoneFactory
     */
    public void setPhoneId(String phoneId) {
        m_phoneId = phoneId;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }
    
    public SettingSet getSettings() {
        return m_settings;
    }
    
    public void setSettings(SettingSet settings) {
        m_settings = settings;
    }
}
