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

public class UserGroup {

    private int m_id;

    private int m_rcsId;

    private int m_orgId;

    private String m_name;

    private int m_parentPgId;

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

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int org_id) {
        this.m_orgId = org_id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

    public int getParentPgId() {
        return m_parentPgId;
    }

    public void setParentPgId(int parent_pg_id) {
        this.m_parentPgId = parent_pg_id;
    }

}

