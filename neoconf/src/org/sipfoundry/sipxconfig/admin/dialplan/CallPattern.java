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
package org.sipfoundry.sipxconfig.admin.dialplan;


/**
 * CallPattern
 */
public class CallPattern {
    private CallDigits m_digits;
    private String m_prefix;

    public CallDigits getDigits() {
        return m_digits;
    }

    public void setDigits(CallDigits digits) {
        m_digits = digits;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public String calculatePattern() {
        String digits = "{" + m_digits.getName() + "}";
        if (m_digits.equals(CallDigits.NO_DIGITS)) {
            digits = "";
        }
        return m_prefix + digits;
    }
}
