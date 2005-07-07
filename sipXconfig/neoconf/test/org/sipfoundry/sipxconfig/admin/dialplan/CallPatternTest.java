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
        pattern.setDigits(CallDigits.NO_DIGITS);
        assertEquals("91", pattern.calculatePattern());

        pattern.setPrefix("12");
        pattern.setDigits(CallDigits.VARIABLE_DIGITS);
        assertEquals("12{vdigits}", pattern.calculatePattern());

        pattern.setPrefix("");
        pattern.setDigits(CallDigits.FIXED_DIGITS);
        assertEquals("{digits}", pattern.calculatePattern());
        
        pattern.setPrefix(null);
        pattern.setDigits(CallDigits.NO_DIGITS);
        assertEquals("", pattern.calculatePattern());
        
        CallPattern pattern2 = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        assertEquals("15{vdigits}", pattern2.calculatePattern());
    }
    
    public void testTransformVariable() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());        
    }

    public void testTransformFixed() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.FIXED_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("1533", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());        
    }

    public void testTransformNoDigits() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.NO_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(0, tdp.getDigits());        
    }    
}
