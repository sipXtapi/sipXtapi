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

import java.util.Date;

import junit.framework.TestCase;

public class DailyBackupScheduleTest extends TestCase {

    public void testGetTimeOfDay() {
        Date midnight = new DailyBackupSchedule().getTimeOfDay();
        String actual = DailyBackupSchedule.TIME_OF_DAY_FORMAT.format(midnight);
        assertEquals("12:00 AM", actual);
    }

}
