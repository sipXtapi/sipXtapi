/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

/**
 * Describing the files required to track and manage a vendor's firmware files
 */
public class Firmware extends BeanWithSettings {
    private String m_name;
    private String m_description;
    private String m_uploadDirectory;
    public String getDescription() {
        return m_description;
    }
    public void setDescription(String description) {
        m_description = description;
    }
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }    
    public String getLabel() {
        return (m_name != null ? m_name : getSettingModel().getName());
    }
    public String getUploadDirectory() {
        return m_uploadDirectory;
    }
    public void setUploadDirectory(String uploadDirectory) {
        m_uploadDirectory = uploadDirectory;
    }
}
