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
package org.sipfoundry.sipxconfig.core;

public class User {

    private int m_id;

    private String m_firstName;

    private int m_orgId;

    private String m_password;

    private int m_ugId;

    private int m_rcsId;

    private String m_lastName;

    private String m_displayId;

    private String m_extension;

    private String m_profileEncryptionKey;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String first_name) {
        this.m_firstName = first_name;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int org_id) {
        this.m_orgId = org_id;
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        this.m_password = password;
    }

    public int getUgId() {
        return m_ugId;
    }

    public void setUgId(int ug_id) {
        this.m_ugId = ug_id;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcs_id) {
        this.m_rcsId = rcs_id;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String last_name) {
        this.m_lastName = last_name;
    }

    public String getDisplayId() {
        return m_displayId;
    }

    public void setDisplayId(String display_id) {
        this.m_displayId = display_id;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        this.m_extension = extension;
    }

    public String getProfileEncryptionKey() {
        return m_profileEncryptionKey;
    }

    public void setProfileEncryptionKey(String profile_encryption_key) {
        this.m_profileEncryptionKey = profile_encryption_key;
    }

}

