/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

public class Location {
    private String m_id;
    private String m_processMonitorUrl;
    private String m_replicationUrl;
    private String m_sipDomain;

    public String getId() {
        return m_id;
    }

    public void setId(String id) {
        m_id = id;
    }

    public String getProcessMonitorUrl() {
        return m_processMonitorUrl;
    }

    public void setProcessMonitorUrl(String processMonitorUrl) {
        m_processMonitorUrl = processMonitorUrl;
    }

    public String getReplicationUrl() {
        return m_replicationUrl;
    }

    public void setReplicationUrl(String replicationUrl) {
        m_replicationUrl = replicationUrl;
    }

    public String getSipDomain() {
        return m_sipDomain;
    }

    public void setSipDomain(String sipDomain) {
        m_sipDomain = sipDomain;
    }
}
