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
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.sql.Timestamp;
import java.util.Date;

import junit.framework.TestCase;

public class HolidayTest extends TestCase {
    private Date m_now;
    private Date m_then;

    protected void setUp() throws Exception {
        m_now = new Date();
        m_then = new Timestamp(0);
    }

    public void testAddDay() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        holiday.addDay(m_now);
        holiday.addDay(m_then);
        holiday.addDay(m_now);
        assertEquals(2, holiday.getDates().size());
        assertTrue(holiday.getDates().contains(m_now));
        assertTrue(holiday.getDates().contains(m_then));
    }

    public void testRemoveDay() {
        Holiday holiday = new Holiday();
        holiday.addDay(m_now);
        assertEquals(1, holiday.getDates().size());
        holiday.removeDay(m_then);
        assertEquals(1, holiday.getDates().size());
        holiday.removeDay(m_now);
        assertTrue(holiday.getDates().isEmpty());
    }

}
