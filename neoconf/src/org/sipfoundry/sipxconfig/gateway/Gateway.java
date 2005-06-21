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
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.phone.AbstractData;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Gateway
 */
public class Gateway extends AbstractData {
    private String m_name;

    private String m_address;

    private String m_description;

    private Setting m_settings;

    private String m_factoryId;

    private String m_serialNumber;

    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel() {
        return m_settings;
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        return m_settings;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
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

    public String getFactoryId() {
        return m_factoryId;
    }

    public void setFactoryId(String factoryId) {
        m_factoryId = factoryId;
    }
}
