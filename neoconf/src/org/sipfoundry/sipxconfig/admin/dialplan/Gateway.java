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
package org.sipfoundry.sipxconfig.admin.dialplan;

/**
 * Gateway
 */
public class Gateway {
    private String m_name;
    private String m_address;
    private String m_description;

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

    public boolean equals(Object o) {
        Gateway other = (Gateway) o;
        if (other == null) {
            return false;
        }
        return m_name.equals(other.m_name);
    }

    public int hashCode() {
        return m_name.hashCode();
    }
}
