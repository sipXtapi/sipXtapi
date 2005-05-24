/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import junit.framework.TestCase;

public class DailyBackupScheduleTest extends TestCase {
    
    private DailyBackupSchedule schedule;
    
    protected void setUp() {
        schedule = new DailyBackupSchedule();
    }

    public void testGetTimeOfDay() {
        Date midnight = schedule.getTimeOfDay();
        String actual = DailyBackupSchedule.GMT_TIME_OF_DAY_FORMAT.format(midnight);
        assertEquals("12:00 AM", actual);
        
        assertEquals(0, schedule.getTimeOfDay().getTime()); // midnight gmt
    }
    
    public void testGetTimerPeriod() {
        assertEquals(1000 * 60 * 60 * 24, schedule.getTimerPeriod());
        schedule.setScheduledDay(ScheduledDay.THURSDAY);
        assertEquals(1000 * 60 * 60 * 24 * 7, schedule.getTimerPeriod());        
    }
    
    public void testGetTimerDate() {
        assertNotNull(schedule.getTimerDate());
        DateFormat localTimeFormat = SimpleDateFormat.getTimeInstance(SimpleDateFormat.LONG); 
        Calendar midnightLocal = Calendar.getInstance();
        midnightLocal.set(Calendar.HOUR_OF_DAY, 0);
        midnightLocal.set(Calendar.MINUTE, 0);
        midnightLocal.set(Calendar.SECOND, 0);
        midnightLocal.set(Calendar.MILLISECOND, 0);
        // midnight local time
        String expected = localTimeFormat.format(midnightLocal.getTime());
        String actual = localTimeFormat.format(schedule.getTimerDate());  
        assertEquals(expected, actual);
    }    
}
