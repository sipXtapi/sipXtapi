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

import junit.framework.TestCase;

/**
 * CallPatternTest
 */
public class CallPatternTest extends TestCase {

    public void testCalculatePattern() {
        CallPattern pattern = new CallPattern();
        pattern.setPrefix("91");
        pattern.setDigits(CallPattern.Digits.NO_DIGITS);        
        assertEquals("91", pattern.calculatePattern());

        pattern.setPrefix("12");
        pattern.setDigits(CallPattern.Digits.VARIABLE_DIGITS);
        assertEquals("12{vdigits}", pattern.calculatePattern());
        
        pattern.setPrefix("");
        pattern.setDigits(CallPattern.Digits.FIXED_DIGITS);
        assertEquals("{digits}", pattern.calculatePattern());
        
    }

}
