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
package org.sipfoundry.sipxconfig.admin.forwarding;

/**
 * AliasMapping
 */
public class AliasMapping {
    private String m_identity;
    private String m_contact;

    public AliasMapping() {
        // empty default
    }

    /**
     * @param identity
     * @param contact
     */
    AliasMapping(String identity, String contact) {
        this.m_identity = identity;
        this.m_contact = contact;
    }

    public synchronized String getContact() {
        return m_contact;
    }

    public synchronized void setContact(String contact) {
        this.m_contact = contact;
    }

    public synchronized String getIdentity() {
        return m_identity;
    }

    public synchronized void setIdentity(String identity) {
        this.m_identity = identity;
    }
}
