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

public class ConfigurationSet {

    private int m_id;

    private int m_rcsId;

    private int m_profileType;

    private String m_content;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcs_id) {
        this.m_rcsId = rcs_id;
    }

    public int getProfileType() {
        return m_profileType;
    }

    public void setProfileType(int profile_type) {
        this.m_profileType = profile_type;
    }

    public String getContent() {
        return m_content;
    }

    public void setContent(String content) {
        this.m_content = content;
    }

}

