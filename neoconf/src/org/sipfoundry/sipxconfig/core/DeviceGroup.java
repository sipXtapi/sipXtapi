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

/**
 * Database object
 */
public class DeviceGroup {

    private int m_id;

    private int m_rcsId;

    private int m_orgId;

    private String m_name;

    private int m_parentPgId;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcsId) {
        m_rcsId = rcsId;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int orgId) {
        m_orgId = orgId;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public int getParentPgId() {
        return m_parentPgId;
    }

    public void setParentPgId(int parentPgId) {
        m_parentPgId = parentPgId;
    }

}
