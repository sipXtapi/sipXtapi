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


/**
 * Final output format
 * <pre>
 * &lt;prefs&gt;
 *   &lt;activegreeting&gt;outofoffice&lt;/activegreeting&gt;
 *   &lt;notification&gt;
 *       &lt;contact type="email" attachments="no"&gt;dhubler@pingtel.com&lt;/contact&gt;
 *   &lt;/notification&gt;
 * &lt;/prefs&gt;
 * </pre>
 */
public class MailboxPreferences {
    public static final String EMAIL_PROP = "emailAddress";
    private ActiveGreeting m_activeGreeting = ActiveGreeting.NONE;
    private String m_emailAddress;
    private boolean m_attachVoicemailToEmail;
    
    public enum ActiveGreeting {
        NONE("none"), 
        STANDARD("standard"), 
        OUT_OF_OFFICE("outofoffice"), 
        EXTENDED_ABSENCE("extendedabsence");
        
        private String m_id;
        
        ActiveGreeting(String id) {
            m_id = id;
        }
        
        public String getId() {
            return m_id;
        }
        
        public static ActiveGreeting valueOfById(String id) {
            for (ActiveGreeting greeting : ActiveGreeting.values()) {
                if (greeting.getId().equals(id)) {
                    return greeting;
                }
            }
            throw new IllegalArgumentException("id not recognized " + id);
        }
    }
    
    public ActiveGreeting getActiveGreeting() {
        return m_activeGreeting;
    }
    
    public void setActiveGreeting(ActiveGreeting activeGreeting) {
        m_activeGreeting = activeGreeting;
    }
    
    public boolean isAttachVoicemailToEmail() {
        return m_attachVoicemailToEmail;
    }
    
    public void setAttachVoicemailToEmail(boolean attachVoicemailToEmail) {
        m_attachVoicemailToEmail = attachVoicemailToEmail;
    }
    
    public String getEmailAddress() {
        return m_emailAddress;
    }
    
    public void setEmailAddress(String emailAddress) {
        m_emailAddress = emailAddress;
    }
}