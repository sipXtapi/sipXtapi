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

public class Patch {

    private int m_patchNumber;

    private Date m_dateApplied;

    private String m_status;

    private String m_description;

    public int getPatchNumber() {
        return m_patchNumber;
    }

    public void setPatchNumber(int patch_number) {
        this.m_patchNumber = patch_number;
    }

    public Date getDateApplied() {
        return m_dateApplied;
    }

    public void setDateApplied(Date date_applied) {
        this.m_dateApplied = date_applied;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        this.m_status = status;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

}

