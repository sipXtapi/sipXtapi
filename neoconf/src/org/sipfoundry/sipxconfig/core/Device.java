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
        this.m_id = id;
    }

    public String getShortName() {
        return m_shortName;
    }

    public void setShortName(String short_name) {
        this.m_shortName = short_name;
    }

    public int getCsdId() {
        return m_csdId;
    }

    public void setCsdId(int csd_id) {
        this.m_csdId = csd_id;
    }

    public int getOrgId() {
        return m_orgId;
    }

    public void setOrgId(int org_id) {
        this.m_orgId = org_id;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcs_id) {
        this.m_rcsId = rcs_id;
    }

    public int getPgId() {
        return m_pgId;
    }

    public void setPgId(int pg_id) {
        this.m_pgId = pg_id;
    }

    public String getUsrsId() {
        return m_usrsId;
    }

    public void setUsrsId(String usrs_id) {
        this.m_usrsId = usrs_id;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int pt_id) {
        this.m_ptId = pt_id;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serial_number) {
        this.m_serialNumber = serial_number;
    }

}

