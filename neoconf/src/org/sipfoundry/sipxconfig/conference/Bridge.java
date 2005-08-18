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
package org.sipfoundry.sipxconfig.conference;

import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

public class Bridge extends BeanWithSettings {
    public static final String BEAN_NAME = "conferenceBridge";

    private boolean m_enabled;

    private String m_name;

    private String m_description;
    
    private Set m_conferences = new HashSet();
    
    public void insertConference(Conference conference) {
        getConferences().add(conference);
        conference.setBridge(this);
    }
    
    public void removeConference(Conference conference) {
        conference.setBridge(null);
        getConferences().remove(conference);
    }    
    
    // trivial get/set
    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }
    
    public Set getConferences() {
        return m_conferences;
    }
    
    public void setConferences(Set conferences) {
        m_conferences = conferences;
    }
}
