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
        m_id = id;
    }

    public String getAdmGrpsId() {
        return m_admGrpsId;
    }

    public void setAdmGrpsId(String admGrpsId) {
        m_admGrpsId = admGrpsId;
    }

    public String getUsrsId() {
        return m_usrsId;
    }

    public void setUsrsId(String usrsId) {
        m_usrsId = usrsId;
    }

    public int getLogPnId() {
        return m_logPnId;
    }

    public void setLogPnId(int logPnId) {
        m_logPnId = logPnId;
    }

    public int getUgId() {
        return m_ugId;
    }

    public void setUgId(int ugId) {
        m_ugId = ugId;
    }

    public int getPgId() {
        return m_pgId;
    }

    public void setPgId(int pgId) {
        m_pgId = pgId;
    }

}
