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
        m_id = id;
    }

    public String getProjectionClass() {
        return m_projectionClass;
    }

    public void setProjectionClass(String projectionClass) {
        m_projectionClass = projectionClass;
    }

    public int getCsdId() {
        return m_csdId;
    }

    public void setCsdId(int csdId) {
        m_csdId = csdId;
    }

    public String getFileName() {
        return m_fileName;
    }

    public void setFileName(String fileName) {
        m_fileName = fileName;
    }

    public String getRenderClass() {
        return m_renderClass;
    }

    public void setRenderClass(String renderClass) {
        m_renderClass = renderClass;
    }

    public int getProfileType() {
        return m_profileType;
    }

    public void setProfileType(int profileType) {
        m_profileType = profileType;
    }

    public String getXsltUrl() {
        return m_xsltUrl;
    }

    public void setXsltUrl(String xsltUrl) {
        m_xsltUrl = xsltUrl;
    }

}
