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
package org.sipfoundry.sipxconfig.admin.intercom;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * The Intercom class represents an intercom (auto-answer) configuration.
 */
// BeanWithGroups is overkill here because it inherits from BeanWithSettings, which
// Intercom isn't.  But there is no easy way to fix that, given that Java doesn't
// support mixins or multiple inheritance of implementations.
public class Intercom extends BeanWithGroups {

    private boolean m_enabled;
    private String m_prefix;
    private int m_timeout;
    private String m_code;    
    
    @Override
    public void initialize() {
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }

    /** Return the code that identifies an auto-answer configuration to the phone */
    public String getCode() {
        return m_code;
    }

    /** Set the code that identifies an auto-answer configuration to the phone */
    public void setCode(String code) {
        m_code = code;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    /** Return the prefix used to place intercom calls */
    public String getPrefix() {
        return m_prefix;
    }

    /** Set the prefix used to place intercom calls */
    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    /** Return the timeout (milliseconds) after which the phone auto-answers */
    public int getTimeout() {
        return m_timeout;
    }

    /** Set the timeout (milliseconds) after which the phone auto-answers */
    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }

}
