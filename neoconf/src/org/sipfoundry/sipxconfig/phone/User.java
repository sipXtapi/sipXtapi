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
import java.util.Set;

/**
 * Database object
 */
public class User implements Serializable {

    private static final long serialVersionUID = 1L;

    private int m_id = -1;

    private String m_firstName;

    private Organization m_org;

    private String m_password;

    private int m_ugId;

    private int m_rcsId;

    private String m_lastName;

    private String m_displayId;

    private String m_extension;

    private String m_profileEncryptionKey;

    private Set m_lines;

    private Set m_assignedEndpoints;

    /**
     * @return Returns the assignedEndpoints.
     */
    public Set getAssignedEndpoints() {
        return m_assignedEndpoints;
    }

    /**
     * @param assignedEndpoints The assignedEndpoints to set.
     */
    public void setAssignedEndpoints(Set assignedEndpoints) {
        m_assignedEndpoints = assignedEndpoints;
    }

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

    public Organization getOrganization() {
        return m_org;
    }

    public void setOrganization(Organization org) {
        m_org = org;
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

    public Set getLines() {
        return m_lines;
    }

    public void setLines(Set lines) {
        m_lines = lines;
    }
}