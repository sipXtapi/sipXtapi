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
 * Database object representing an actualy physical phone you can touch.
 */
public class Endpoint {

    private int m_id = -1;

    private String m_name;

    private String m_description;

    private String m_serialNumber;

    private User m_user;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
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
    
    /**
     * @return null if unassigned
     */
    public User getUser() {
        return m_user;
    }
    /**
     * @param user The user to set.
     */
    public void setUser(User user) {
        m_user = user;
    }
}
