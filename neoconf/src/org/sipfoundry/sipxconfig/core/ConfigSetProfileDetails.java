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

public class ConfigSetProfileDetails {

    private int m_id;

    private String m_projectionClass;

    private int m_csdId;

    private String m_fileName;

    private String m_renderClass;

    private int m_profileType;

    private String m_xsltUrl;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public String getProjectionClass() {
        return m_projectionClass;
    }

    public void setProjectionClass(String projection_class) {
        this.m_projectionClass = projection_class;
    }

    public int getCsdId() {
        return m_csdId;
    }

    public void setCsdId(int csd_id) {
        this.m_csdId = csd_id;
    }

    public String getFileName() {
        return m_fileName;
    }

    public void setFileName(String file_name) {
        this.m_fileName = file_name;
    }

    public String getRenderClass() {
        return m_renderClass;
    }

    public void setRenderClass(String render_class) {
        this.m_renderClass = render_class;
    }

    public int getProfileType() {
        return m_profileType;
    }

    public void setProfileType(int profile_type) {
        this.m_profileType = profile_type;
    }

    public String getXsltUrl() {
        return m_xsltUrl;
    }

    public void setXsltUrl(String xslt_url) {
        this.m_xsltUrl = xslt_url;
    }

}

