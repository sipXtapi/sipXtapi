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

public class Extension {

    private int m_extensionNumber;

    private int m_extPoolId;

    private String m_status;

    public int getExtensionNumber() {
        return m_extensionNumber;
    }

    public void setExtensionNumber(int extension_number) {
        this.m_extensionNumber = extension_number;
    }

    public int getExtPoolId() {
        return m_extPoolId;
    }

    public void setExtPoolId(int ext_pool_id) {
        this.m_extPoolId = ext_pool_id;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        this.m_status = status;
    }

}

