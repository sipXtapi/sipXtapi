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

import org.apache.commons.lang.builder.CompareToBuilder;

public class RegistrationItem implements Comparable {
    private String m_uri;
    private String m_contact;
    private long m_expires;
    private String m_primary;

    public String getContact() {
        return m_contact;
    }

    public void setContact(String contact) {
        m_contact = contact;
    }

    public long getExpires() {
        return m_expires;
    }

    public void setExpires(long expires) {
        m_expires = expires;
    }

    public String getUri() {
        return m_uri;
    }

    public void setUri(String uri) {
        m_uri = uri;
    }
    
    public String getPrimary() {
        return m_primary;
    }
    
    public void setPrimary(String primary) {
        m_primary = primary;
    }

    public int compareTo(Object other) {
        RegistrationItem riOther = (RegistrationItem) other;
        return new CompareToBuilder().append(getExpires(), riOther.getExpires()).toComparison();
    }

    public long timeToExpireAsSeconds(long nowSeconds) {
        return getExpires() - nowSeconds;
    }
}
