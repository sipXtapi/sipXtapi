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

import org.apache.commons.lang.ObjectUtils;
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
        m_prefix = (String) ObjectUtils.defaultIfNull(prefix, StringUtils.EMPTY);
    }

    public String calculatePattern() {
        String digits = "{" + m_digits.getName() + "}";
        if (m_digits.equals(CallDigits.NO_DIGITS)) {
            digits = StringUtils.EMPTY;
        }
        return m_prefix + digits;
    }

    /**
     * Transforms dial pattern according to call pattern setting
     * 
     * @param from dial pattern to transform
     * @return resulting dial pattern
     */
    public DialPattern transform(DialPattern from) {
        DialPattern to = new DialPattern();
        if (m_digits.equals(CallDigits.FIXED_DIGITS)) {
            to.setPrefix(m_prefix + from.getPrefix());
        } else {
            to.setPrefix(m_prefix);
        }
        if (m_digits.equals(CallDigits.NO_DIGITS)) {
            to.setDigits(0);
        } else {
            to.setDigits(from.getDigits());
        }
        return to;
    }
}
