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
package org.sipfoundry.sipxconfig.admin;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;

public class WhackerTest extends TestCase {
    private Whacker m_whacker;
    
    protected void setUp() throws Exception {
        m_whacker = new Whacker();
    }

    public void testWhacker() throws Exception {
        // The Whacker is supposed to do a restart through the processContext.
        // Make a mock control that checks that.
        MockControl processControl = MockClassControl.createStrictControl(SipxProcessContext.class);
        SipxProcessContext processContext = (SipxProcessContext) processControl.getMock();
        processContext.manageServices(Whacker.SERVICE_NAMES, SipxProcessContext.Command.RESTART);
        processControl.replay();
        m_whacker.setProcessContext(processContext);

        // Set the WhackerTask to run very soon so we don't get bored waiting for it
        Date date = new Date();
        date.setTime(date.getTime() + 500);    // add 1/2 second
        DateFormat df = new SimpleDateFormat(Whacker.TEST_DATE_FORMAT);
        m_whacker.setTimeOfDay(df.format(date));
        
        // Run the Whacker, simulating app startup
        m_whacker.setEnabled(true);            // in case it is disabled via properties file
        m_whacker.onApplicationEvent(new ApplicationInitializedEvent(this));
        
        // Wait for a second to make sure the task has run, then verify
        Thread.sleep(1000);
        processControl.verify();
    }

    public void testWhackerLikesDateFormat() {
        // testWhacker above uses a special date format.  Make sure that the typical date format works.
        // Offset the time by one hour from now just to make sure the timer task won't fire, we're just
        // checking that the date works OK.
        Date date = new Date();
        date.setTime(date.getTime() + 1000 * 60 * 60);
        DateFormat df = DateFormat.getTimeInstance(DateFormat.SHORT);
        m_whacker.setTimeOfDay(df.format(date));
        m_whacker.setEnabled(true);            // in case it is disabled via properties file
        m_whacker.onApplicationEvent(new ApplicationInitializedEvent(this));
    }
    
    public void testGetScheduledDay() {
        String dayNames[] = new String[] {"Every day", "Sunday", "Monday", "Tuesday", "Wednesday",
                "Thursday", "Friday", "Saturday"};
        ScheduledDay[] days = new ScheduledDay[] {ScheduledDay.EVERYDAY, ScheduledDay.SUNDAY,
                ScheduledDay.MONDAY, ScheduledDay.TUESDAY, ScheduledDay.WEDNESDAY,
                ScheduledDay.THURSDAY, ScheduledDay.FRIDAY, ScheduledDay.SATURDAY};
        for (int i = 0; i < days.length; i++) {
            m_whacker.setScheduledDay(dayNames[i]);
            assertTrue(m_whacker.getScheduledDayEnum() == days[i]);
        }
    }
    
}
