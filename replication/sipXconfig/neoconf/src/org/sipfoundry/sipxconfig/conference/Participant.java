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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

public class Participant extends BeanWithSettings {
    public static final String BEAN_NAME = "conferenceParticipant";

    private boolean m_enabled;

    private Conference m_conference;
    
    private User m_user;

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public Conference getConference() {
        return m_conference;
    }

    public void setConference(Conference conference) {
        m_conference = conference;
    }
    
    public User getUser() {
        return m_user;
    }
    
    public void setUser(User user) {
        m_user = user;
    }
}
