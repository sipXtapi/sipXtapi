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
public class Device {

    private int m_id;

    private String m_shortName;

    private int m_csdId;

    private int m_orgId;

    private int m_rcsId;

    private int m_pgId;

    private String m_usrsId;

    private String m_description;

    private int m_ptId;

    private String m_serialNumber;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getShortName() {
        return m_shortName;
    }

    public void setShortName(String shortName) {
        m_shortName = shortName;
    }

    public int getCsdId() {
        return m_csdId;
    }

    public void setCsdId(int csdId) {
        m_csdId = csdId;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int orgId) {
        m_orgId = orgId;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcsId) {
        m_rcsId = rcsId;
    }

    public int getPgId() {
        return m_pgId;
    }

    public void setPgId(int pgId) {
        m_pgId = pgId;
    }

    public String getUsrsId() {
        return m_usrsId;
    }

    public void setUsrsId(String usrsId) {
        m_usrsId = usrsId;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int ptId) {
        m_ptId = ptId;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

}
