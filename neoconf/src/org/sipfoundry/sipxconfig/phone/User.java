/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;

/**
 * User that logs in, and base info for most lines
 */
public class User implements Serializable {

    private static final long serialVersionUID = 1L;

    private int m_id = PhoneContext.UNSAVED_ID;

    private String m_firstName;

    private Organization m_organization;

    private String m_password;

    private int m_ugId = 1; //default group

    private int m_rcsId = 2; // 2='Complete User'

    private String m_lastName;

    private String m_displayId;

    private String m_extension;

    private String m_profileEncryptionKey;

    private Credential m_credential;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }

    public int getUserGroupId() {
        return m_ugId;
    }

    public void setUserGroupId(int ugId) {
        m_ugId = ugId;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcsId) {
        m_rcsId = rcsId;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getDisplayId() {
        return m_displayId;
    }

    public void setDisplayId(String displayId) {
        m_displayId = displayId;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getProfileEncryptionKey() {
        return m_profileEncryptionKey;
    }

    public void setProfileEncryptionKey(String profileEncryptionKey) {
        m_profileEncryptionKey = profileEncryptionKey;
    }

    public Organization getOrganization() {
        return m_organization;
    }

    public void setOrganization(Organization organization) {
        m_organization = organization;
    }

    public int getUgId() {
        return m_ugId;
    }

    public void setUgId(int ugId) {
        m_ugId = ugId;
    }

    public Credential getCredential() {
        if (m_credential == null) {
            m_credential = new Credential();
            m_credential.setAuthId(getDisplayId());
            m_credential.setPassword(getPassword());
            m_credential.setRealm(m_organization.getDnsDomain());            
        }
        
        return m_credential;
    }

    public void setCredential(Credential credential) {
        m_credential = credential;
    }
}