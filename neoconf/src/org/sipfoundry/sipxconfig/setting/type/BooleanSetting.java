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

/**
 * Setting is true or false, If you need true, false and not specified, use
 * EnumSetting.
 */
public class BooleanSetting implements SettingType {

    private String m_true = "1";

    private String m_false = "0";

    public String getName() {
        return "boolean";
    }

    public String getFalse() {
        return m_false;
    }

    public void setFalse(String false1) {
        m_false = false1;
    }

    public String getTrue() {
        return m_true;
    }

    public void setTrue(String true1) {
        m_true = true1;
    }
}
