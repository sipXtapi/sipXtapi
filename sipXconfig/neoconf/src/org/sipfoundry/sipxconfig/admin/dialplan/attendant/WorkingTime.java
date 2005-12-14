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

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;

public class WorkingTime extends ScheduledAttendant {
    public static final int LAST_WORKING_DAY = 5;

    private WorkingHours[] m_workingHours;

    public WorkingTime() {
        m_workingHours = new WorkingHours[ScheduledDay.DAYS_OF_WEEK.length];
        for (int i = 0; i < m_workingHours.length; i++) {
            WorkingHours whs = new WorkingHours();
            whs.setDay(ScheduledDay.DAYS_OF_WEEK[i]);
            if (i < LAST_WORKING_DAY) {
                whs.setEnabled(true);
            }
            m_workingHours[i] = whs;
        }
    }

    public WorkingHours[] getWorkingHours() {
        return m_workingHours;
    }

    public void setWorkingHours(WorkingHours[] workingHours) {
        m_workingHours = workingHours;
    }

    public static class WorkingHours {
        public static final int DEFAULT_START = 9;
        public static final int DEFAULT_STOP = 18;

        public static final DateFormat TIME_FORMAT = new SimpleDateFormat("HH:mm", Locale.US);

        private boolean m_enabled;
        private ScheduledDay m_day;
        private Date m_start;
        private Date m_stop;

        public WorkingHours() {
            Calendar calendar = Calendar.getInstance();
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_START);
            calendar.set(Calendar.MINUTE, 0);
            m_start = calendar.getTime();
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_STOP);
            m_stop = calendar.getTime();
        }

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

        public Date getStart() {
            return m_start;
        }

        public void setStart(Date start) {
            m_start = start;
        }

        public Date getStop() {
            return m_stop;
        }

        public void setStop(Date stop) {
            m_stop = stop;
        }
    }
}
