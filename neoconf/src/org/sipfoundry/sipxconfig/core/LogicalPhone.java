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
public class LogicalPhone {

    // TODO: Core Software Details, see if this is still used
    //protected int m_csdId = 1;

    // TODO: Ref Config Set, see if this is still used
    //protected int m_rcsId = 1;

    // TODO: Phone Group, see if this is still used
    //protected int m_pgId = 1;

    // TODO: Phone Type, see if this is still used
    //protected int m_ptId = 1;

    private int m_id = -1;

    private String m_shortName;

    private String m_description;

    private String m_serialNumber;

    private User m_user;
    
    private Organization m_org;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getShortName() {
        return m_shortName;
    }

    public void setShortName(String shortName) {
        m_shortName = shortName;
    }

    public Organization getOrganization() {
        return m_org;
    }

    public void setOrganization(Organization org) {
        m_org = org;
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
