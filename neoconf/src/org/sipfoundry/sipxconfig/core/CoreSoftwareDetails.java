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

import java.util.Date;

public class CoreSoftwareDetails {

    private int m_id;

    private Date m_installedDate;

    private String m_description;

    private String m_version;

    private String m_name;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public Date getInstalledDate() {
        return m_installedDate;
    }

    public void setInstalledDate(Date installed_date) {
        this.m_installedDate = installed_date;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

    public String getVersion() {
        return m_version;
    }

    public void setVersion(String version) {
        this.m_version = version;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

}

