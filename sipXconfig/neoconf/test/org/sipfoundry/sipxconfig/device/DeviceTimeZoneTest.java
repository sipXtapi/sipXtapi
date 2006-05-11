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
package org.sipfoundry.sipxconfig.device;

import junit.framework.TestCase;

public class DeviceTimeZoneTest extends TestCase {
    
    public void testGetOffset() {
        DeviceTimeZone mytz = new DeviceTimeZone("Europe/Helsinki");
        
        int ofs = mytz.getOffset();
        assertEquals(7200, ofs);

        ofs = mytz.getDstOffset();
        assertEquals(3600, ofs);

        String actual = mytz.getShortName();
        assertEquals("EET", actual);
    }

    public void testDST() {
        DeviceTimeZone mytz = new DeviceTimeZone("Europe/Helsinki");
        
        int value = mytz.getStartDayOfWeek();
        assertEquals(1, value);

        value = mytz.getStartTime();
        assertEquals(3 * 3600, value);

        value = mytz.getStopTime();
        assertEquals(4 * 3600, value);

        value = mytz.getStopWeek();
        assertEquals(DeviceTimeZone.DST_LASTWEEK, value);

        mytz.setDstRule(DeviceTimeZone.DST_US);

        value = mytz.getStartTime();
        assertEquals(2 * 3600, value);
    }
}
