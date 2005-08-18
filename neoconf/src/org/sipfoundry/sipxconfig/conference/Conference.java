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

public class Conference extends BeanWithSettings {
    public static final String BEAN_NAME = "conferenceConference";

    private boolean m_enabled;

    private String m_name;

    private String m_description;
    
    private String m_extension;
    
    private Bridge m_bridge;

    private Set m_participants = new HashSet();
        
    public void insertParticipant(Participant participant) {
        getParticipants().add(participant);
        participant.setConference(this);
    }
    
    public void removeParticipant(Participant participant) {
        participant.setConference(null);
        getParticipants().remove(participant);
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
    
    public Bridge getBridge() {
        return m_bridge;
    }
    
    public void setBridge(Bridge bridge) {
        m_bridge = bridge;
    }

    public Set getParticipants() {
        return m_participants;
    }
    
    public void setParticipants(Set participants) {
        m_participants = participants;
    }
    
    public String getExtension() {
        return m_extension;
    }
    
    public void setExtension(String extension) {
        m_extension = extension;
    }
}
