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

public class Version {

    private Date m_applied;

    private int m_version;

    public Date getApplied() {
        return m_applied;
    }

    public void setApplied(Date applied) {
        this.m_applied = applied;
    }

    public int getVersion() {
        return m_version;
    }

    public void setVersion(int version) {
        this.m_version = version;
    }

}

