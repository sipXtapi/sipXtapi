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
package org.sipfoundry.sipxconfig.setting;

public class ConditionalSettingImpl extends SettingImpl implements ConditionalSetting {
    private String m_if;
    private String m_unless;
    public String getIf() {
        return m_if;
    }
    public void setIf(String if1) {
        m_if = if1;
    }
    public String getUnless() {
        return m_unless;
    }
    public void setUnless(String unless) {
        m_unless = unless;
    }
}
