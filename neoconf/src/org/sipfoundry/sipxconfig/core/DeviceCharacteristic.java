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

public class DeviceCharacteristic {

    private String m_rpCharsCharacteristic;

    private String m_value;

    private int m_ptId;

    private int m_appsId;

    public String getRpCharsCharacteristic() {
        return m_rpCharsCharacteristic;
    }

    public void setRpCharsCharacteristic(String rp_chars_characteristic) {
        this.m_rpCharsCharacteristic = rp_chars_characteristic;
    }

    public String getValue() {
        return m_value;
    }

    public void setValue(String value) {
        this.m_value = value;
    }

    public int getPtId() {
        return m_ptId;
    }

    public void setPtId(int pt_id) {
        this.m_ptId = pt_id;
    }

    public int getAppsId() {
        return m_appsId;
    }

    public void setAppsId(int apps_id) {
        this.m_appsId = apps_id;
    }

}

