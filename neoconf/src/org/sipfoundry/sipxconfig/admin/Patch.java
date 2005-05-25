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
package org.sipfoundry.sipxconfig.admin;

import java.util.Date;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Patch applied to system, either automatically or initiated by end user.
 */
public class Patch extends BeanWithId {

    public static final char NOT_YET_APPLIED = 'N';
    public static final char ALREADY_APPLIED = 'A';
    public static final char FAILURE = 'F';
    public static final char SUCCESS = 'S';

    private char m_status;

    private Date m_appliedDate;

    private String m_description;

    public String getDescription() {
        return m_description;
    }

    /**
     * @deprecated this is for bean access only.
     */
    public void setDescription(String description) {
        m_description = description;
    }

    public Date getDateApplied() {
        return m_appliedDate;
    }

    public void setDateApplied(Date appliedDate) {
        m_appliedDate = appliedDate;
    }

    public char getStatus() {
        return m_status;
    }

    public void setStatus(char status) {
        m_status = status;
    }
}
