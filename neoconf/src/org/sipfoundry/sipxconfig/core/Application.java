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
public class Application {

    private int m_id;

    private int m_ptId;

    private int m_refPropId;

    private String m_url;

    private int m_orgId;

    private String m_name;

    private String m_description;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int ptId) {
        m_ptId = ptId;
    }

    public int getRefPropId() {
        return m_refPropId;
    }

    public void setRefPropId(int refPropId) {
        m_refPropId = refPropId;
    }

    public String getUrl() {
        return m_url;
    }

    public void setUrl(String url) {
        m_url = url;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int orgId) {
        m_orgId = orgId;
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

}
