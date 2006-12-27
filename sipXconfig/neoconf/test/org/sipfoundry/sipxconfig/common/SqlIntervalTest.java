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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.common.SqlInterval;

public class SqlIntervalTest extends TestCase {

    public void testInterval2string() {
        PGInterval pgInterval = new PGInterval();
        pgInterval.setMinutes(10);
        SqlInterval sqlInterval = new SqlInterval(pgInterval);
        assertEquals(600000, sqlInterval.getMillisecs());
    }
    
    public void testCompareTo() {
        PGInterval pga = new PGInterval();
        pga.setDays(1);
        SqlInterval a = new SqlInterval(pga);
        
        PGInterval pgb = new PGInterval();
        pgb.setDays(2);
        SqlInterval b = new SqlInterval(pgb);
        
        assertEquals(-1, a.compareTo(b));
        assertEquals(1, b.compareTo(a));
        assertEquals(0, a.compareTo(a));       
    }
}
