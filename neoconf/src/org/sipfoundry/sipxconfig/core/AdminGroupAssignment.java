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

public class AdminGroupAssignment {

    private int m_id;

    private String m_admGrpsId;

    private String m_usrsId;

    private int m_logPnId;

    private int m_ugId;

    private int m_pgId;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public String getAdmGrpsId() {
        return m_admGrpsId;
    }

    public void setAdmGrpsId(String adm_grps_id) {
        this.m_admGrpsId = adm_grps_id;
    }

    public String getUsrsId() {
        return m_usrsId;
    }

    public void setUsrsId(String usrs_id) {
        this.m_usrsId = usrs_id;
    }

    public int getLogPnId() {
        return m_logPnId;
    }

    public void setLogPnId(int log_pn_id) {
        this.m_logPnId = log_pn_id;
    }

    public int getUgId() {
        return m_ugId;
    }

    public void setUgId(int ug_id) {
        this.m_ugId = ug_id;
    }

    public int getPgId() {
        return m_pgId;
    }

    public void setPgId(int pg_id) {
        this.m_pgId = pg_id;
    }

}

