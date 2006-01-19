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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

public class PhoneTimeZoneTest extends TestCase {
    
    public void testGetOffset() {
        PhoneTimeZone.setTimeZone("Europe/Helsinki");
        PhoneTimeZone mytz = new PhoneTimeZone();
        
        int ofs = mytz.getOffset();
        assertEquals(7200, ofs);

        ofs = mytz.getDstOffset();
        assertEquals(3600, ofs);

        String actual = mytz.getShortName();
        assertEquals("EET", actual);
    }

    public void testDST() {
        PhoneTimeZone.setTimeZone("Europe/Helsinki");
        PhoneTimeZone mytz = new PhoneTimeZone();
        
        int value = mytz.getStartDayOfWeek();
        assertEquals(1, value);

        value = mytz.getStartTime();
        assertEquals(3 * 3600, value);

        value = mytz.getStopTime();
        assertEquals(4 * 3600, value);

        value = mytz.getStopWeek();
        assertEquals(PhoneTimeZone.DST_LASTWEEK, value);

        mytz.setDstRule(PhoneTimeZone.DST_US);

        value = mytz.getStartTime();
        assertEquals(2 * 3600, value);
    }
}
