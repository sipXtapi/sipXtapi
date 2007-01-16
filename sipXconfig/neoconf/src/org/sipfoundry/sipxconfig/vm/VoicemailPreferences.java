/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vm;

public class VoicemailPreferences {

    private String m_emailAddress;
    
    private String m_activeGreeting;
    
    private boolean m_attachVoicemailToEmail;

    public String getActiveGreeting() {
        return m_activeGreeting;
    }

    public void setActiveGreeting(String activeGreeting) {
        m_activeGreeting = activeGreeting;
    }

    public String getEmailAddress() {
        return m_emailAddress;
    }

    public void setEmailAddress(String emailAddress) {
        m_emailAddress = emailAddress;
    }

    public boolean isAttachVoicemailToEmail() {
        return m_attachVoicemailToEmail;
    }

    public void setAttachVoicemailToEmail(boolean attachVoicemailToEmail) {
        m_attachVoicemailToEmail = attachVoicemailToEmail;
    }
}
