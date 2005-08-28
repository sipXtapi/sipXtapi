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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

public class RegistrationItem {
    private String m_uri;
    private String m_contact;
    private String m_expires;
    public String getContact() {
        return m_contact;
    }
    public void setContact(String contact) {
        m_contact = contact;
    }
    public String getExpires() {
        return m_expires;
    }
    public void setExpires(String expires) {
        m_expires = expires;
    }
    public String getUri() {
        return m_uri;
    }
    public void setUri(String uri) {
        m_uri = uri;
    }

}
