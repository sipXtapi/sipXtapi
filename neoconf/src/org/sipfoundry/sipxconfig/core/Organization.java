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
 * Database object
 */
public class Organization {

    private int m_id;

    private String m_name;

    private int m_parentId;

    private String m_dnsDomain;

    private int m_stereotype;

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

    public int getParentId() {
        return m_parentId;
    }

    public void setParentId(int parentId) {
        m_parentId = parentId;
    }

    public String getDnsDomain() {
        return m_dnsDomain;
    }

    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain;
    }

    public int getStereotype() {
        return m_stereotype;
    }

    public void setStereotype(int stereotype) {
        m_stereotype = stereotype;
    }

}
