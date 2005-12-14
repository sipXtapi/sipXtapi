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
import java.text.DateFormat;
import java.util.Date;
import java.util.Locale;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;

public class WorkingTime extends ScheduledAttendant {
    private WorkingHours[] m_workingHours;

    public static class WorkingHours {
        public static final DateFormat TIME_FORMAT = DateFormat.getTimeInstance(DateFormat.SHORT,
                Locale.US);

        private boolean m_enabled;
        private ScheduledDay m_day;
        private Date m_start = new Timestamp(0);
        private Date m_stop = new Timestamp(0);

        public void setEnabled(boolean enabled) {
            m_enabled = enabled;
        }

        public boolean isEnabled() {
            return m_enabled;
        }

        public void setDay(ScheduledDay day) {
            m_day = day;
        }

        public ScheduledDay getDay() {
            return m_day;
        }

        public String getStartTime() {
            return TIME_FORMAT.format(m_start);
        }

        public String getStopTime() {
            return TIME_FORMAT.format(m_stop);
        }
    }

    public WorkingTime() {
        m_workingHours = new WorkingHours[ScheduledDay.DAYS_OF_WEEK.length];
        for (int i = 0; i < m_workingHours.length; i++) {
            WorkingHours whs = new WorkingHours();
            whs.setDay(ScheduledDay.DAYS_OF_WEEK[i]);
        }
    }
}
