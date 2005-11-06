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
package org.sipfoundry.sipxconfig.domain;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

public class Domain extends BeanWithSettings {    
    private String m_name;
    private String m_description;
    private String m_type;
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
    public String getType() {
        return m_type;
    }
    public void setType(String type) {
        m_type = type;
    }
    public String getBeanId() {
        return "unmanagedBean";
    }
    /** do not use, byproduct of hibernate mapping */
    public void setBeanId(String beanId_) {
    }
}
