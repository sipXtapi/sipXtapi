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

public class RealSetting implements SettingType {
    private float m_min;

    private float m_max = Float.MAX_VALUE;

    private boolean m_required;

    public float getMax() {
        return m_max;
    }

    public void setMax(float max) {
        m_max = max;
    }

    public float getMin() {
        return m_min;
    }

    public void setMin(float min) {
        m_min = min;
    }

    public String getName() {
        return "real";
    }

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }
}
