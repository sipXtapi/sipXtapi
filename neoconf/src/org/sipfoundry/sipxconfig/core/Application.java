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
        this.m_id = id;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int pt_id) {
        this.m_ptId = pt_id;
    }

    public int getRefPropId() {
        return m_refPropId;
    }

    public void setRefPropId(int ref_prop_id) {
        this.m_refPropId = ref_prop_id;
    }

    public String getUrl() {
        return m_url;
    }

    public void setUrl(String url) {
        this.m_url = url;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int org_id) {
        this.m_orgId = org_id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

}

