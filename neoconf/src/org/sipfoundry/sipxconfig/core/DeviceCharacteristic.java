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

/**
 * Database object
 */
public class DeviceCharacteristic {

    private String m_rpCharsCharacteristic;

    private String m_value;

    private int m_ptId;

    private int m_appsId;

    public String getRpCharsCharacteristic() {
        return m_rpCharsCharacteristic;
    }

    public void setRpCharsCharacteristic(String rpCharsCharacteristic) {
        m_rpCharsCharacteristic = rpCharsCharacteristic;
    }

    public String getValue() {
        return m_value;
    }

    public void setValue(String value) {
        m_value = value;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int ptId) {
        m_ptId = ptId;
    }

    public int getAppsId() {
        return m_appsId;
    }

    public void setAppsId(int appsId) {
        m_appsId = appsId;
    }

}
