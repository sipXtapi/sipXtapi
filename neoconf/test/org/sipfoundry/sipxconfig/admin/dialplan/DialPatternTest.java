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
 * DialPatternTest
 */
public class DialPatternTest extends TestCase {
    public void testCalculatePattern() {
        DialPattern pattern = new DialPattern();
        pattern.setDigits(3);
        pattern.setPrefix("19");
        assertEquals("19xxx", pattern.calculatePattern());

        pattern.setDigits(0);
        pattern.setPrefix("19");
        assertEquals("19", pattern.calculatePattern());
        
        pattern.setDigits(5);
        pattern.setPrefix("");
        assertEquals("xxxxx", pattern.calculatePattern());        
    }

}
