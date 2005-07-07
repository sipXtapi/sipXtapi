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
import java.util.StringTokenizer;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Represents a regular expression corresponding to what users dialed. This concrete class allows
 * for building such expression using constant prefix and number of digits in a suffic
 */
public class DialPattern {
    private static final String SEPARATORS = " ,";

    private String m_prefix;
    private int m_digits;

    public DialPattern(String prefix, int digits) {
        m_prefix = prefix;
        m_digits = digits;
    }

    public DialPattern() {
        this("", 0);
    }

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

    /**
     * Helper function to tokenize list of numbers into array of strings
     * 
     * @param patternsList comma separated list of patterns, spaces are treated as separators as well
     * @param suffix string appended to each pattern in the resulting list
     */
    public static String[] getPatternsFromList(String patternsList, String suffix) {
        if (StringUtils.isBlank(patternsList)) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        StringTokenizer tokenizer = new StringTokenizer(patternsList, SEPARATORS);
        String[] patterns = new String[tokenizer.countTokens()];
        for (int i = 0; i < patterns.length; i++) {
            patterns[i] = tokenizer.nextToken() + suffix;
        }
        return patterns;
    }
}
