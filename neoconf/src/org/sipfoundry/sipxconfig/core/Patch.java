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

import java.util.Date;

/**
 * Database object
 */
public class Patch {

    private int m_patchNumber;

    private Date m_dateApplied;

    private String m_status;

    private String m_description;

    public int getPatchNumber() {
        return m_patchNumber;
    }

    public void setPatchNumber(int patchNumber) {
        m_patchNumber = patchNumber;
    }

    public Date getDateApplied() {
        return m_dateApplied;
    }

    public void setDateApplied(Date dateApplied) {
        m_dateApplied = dateApplied;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        m_status = status;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

}
