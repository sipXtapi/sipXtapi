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
package org.sipfoundry.sipxconfig.core;

/**
 * Represents a particular phone model (e.g. Cisco 7960)
 */
public class DeviceModel {

    private int m_id;

    private String m_model;

    private int m_manuId;

    public DeviceModel(String model) {
        setModel(model);
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    /**
     * @return Returns the name (e.g. 7960)
     */
    public String getModel() {
        return m_model;
    }

    /**
     * @param name The name to set. (e.g. 7960)
     */
    public void setModel(String model) {
        m_model = model;
    }

    public int getManuId() {
        return m_manuId;
    }

    public void setManuId(int manuId) {
        m_manuId = manuId;
    }
}
