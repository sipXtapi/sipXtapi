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
package org.sipfoundry.sipxconfig.admin.dialplan;

import static org.sipfoundry.sipxconfig.admin.dialplan.CallDigits.findFirstNonEscapedSpecialChar;
import junit.framework.TestCase;

public class CallDigitsTest extends TestCase {

    public void testFindFirstNonEscapedSpecialChar() {
        assertEquals(0, findFirstNonEscapedSpecialChar("abcd", "ad", 'c'));
        assertEquals(1, findFirstNonEscapedSpecialChar("abcd", "db", 'c'));
        assertEquals(-1, findFirstNonEscapedSpecialChar("abcd", "zx", 'c'));
        assertEquals(-1, findFirstNonEscapedSpecialChar("abcd", "de", 'c'));
        assertEquals(4, findFirstNonEscapedSpecialChar("abccd", "de", 'c'));
    }

    public void testGetEscapedName() {
        assertEquals("digits-escaped", CallDigits.FIXED_DIGITS.getEscapedName());
        assertEquals("vdigits-escaped", CallDigits.VARIABLE_DIGITS.getEscapedName());
    }
}
