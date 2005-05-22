/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

public class IntegerSetting implements SettingType {
    private int m_min;
    private int m_max = Integer.MAX_VALUE;

    public int getMax() {
        return m_max;
    }

    public void setMax(int max) {
        m_max = max;
    }

    public int getMin() {
        return m_min;
    }

    public void setMin(int min) {
        m_min = min;
    }
}
