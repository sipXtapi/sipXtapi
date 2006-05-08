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
package org.sipfoundry.sipxconfig.setting;

public class SettingValueImpl implements SettingValue2 {
    private String m_value;
    
    public SettingValueImpl(String value) {
        m_value = value;
    }

    public String getValue() {
        return m_value;
    }
}
