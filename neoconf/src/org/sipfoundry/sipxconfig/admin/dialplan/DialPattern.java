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

import java.util.Arrays;

/**
 * Represents a regular expression corresponding to what users dialed. This
 * concrete class allows for building such expression using constant prefix and
 * number of digits in a suffic
 */
public class DialPattern {
    private String m_prefix;
    private int m_digits;

    public int getDigits() {
        return m_digits;
    }

    public void setDigits(int digits) {
        m_digits = digits;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public String calculatePattern() {
        char[] fixedDigits = new char[m_digits];
        Arrays.fill(fixedDigits, 'x');
        StringBuffer buf = new StringBuffer(m_prefix);
        buf.append(fixedDigits);
        return buf.toString();
    }
}
