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

import org.apache.commons.lang.StringUtils;

/**
 * CallPattern
 */
public class CallPattern {
    private CallDigits m_digits;
    private String m_prefix;

    public CallPattern() {
        this(StringUtils.EMPTY, CallDigits.NO_DIGITS);
    }

    public CallPattern(String prefix, CallDigits digits) {
        m_prefix = prefix;
        m_digits = digits;
    }

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
            digits = StringUtils.EMPTY;
        }
        return m_prefix + digits;
    }
}
