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

    public void setExtensionNumber(int extensionNumber) {
        m_extensionNumber = extensionNumber;
    }

    public int getExtPoolId() {
        return m_extPoolId;
    }

    public void setExtPoolId(int extPoolId) {
        m_extPoolId = extPoolId;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        m_status = status;
    }

}
