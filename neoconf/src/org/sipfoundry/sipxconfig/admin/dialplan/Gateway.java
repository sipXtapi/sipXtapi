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
    private static int s_id = 1;

    private Integer m_id;
    private String m_name;
    private String m_address;
    private String m_description;

    public Gateway() {
        // TODO: replace by hibernate id handling
        setId(new Integer(s_id++));
    }

    /**
     * Only to be used by DialPlanManager
     * @param id of the existing gateway
     */
    Gateway(Integer id) {
        setId(id);
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

    public Integer getId() {
        return m_id;
    }

    private void setId(Integer id) {
        m_id = id;
    }

    public boolean equals(Object o) {
        Gateway other = (Gateway) o;
        if (other == null) {
            return false;
        }
        return m_id.equals(other.m_id);
    }

    public int hashCode() {
        return m_id.hashCode();
    }

    public void update(Gateway gateway) {
        m_name = gateway.m_name;
        m_address = gateway.m_address;
        m_description = gateway.m_description;
    }
}
