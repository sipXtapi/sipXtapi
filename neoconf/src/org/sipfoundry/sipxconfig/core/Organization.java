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
        this.m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

    public int getParentId() {
        return m_parentId;
    }

    public void setParentId(int parent_id) {
        this.m_parentId = parent_id;
    }

    public String getDnsDomain() {
        return m_dnsDomain;
    }

    public void setDnsDomain(String dns_domain) {
        this.m_dnsDomain = dns_domain;
    }

    public int getStereotype() {
        return m_stereotype;
    }

    public void setStereotype(int stereotype) {
        this.m_stereotype = stereotype;
    }

}

