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
import java.util.TimeZone;

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

    public Object clone() throws CloneNotSupportedException {
        WorkingTime clone = (WorkingTime) super.clone();
        clone.m_workingHours = m_workingHours.clone();
        return clone;
    }

    public static class WorkingHours {
        public static final int DEFAULT_START = 9;
        public static final int DEFAULT_STOP = 18;

        public static final DateFormat TIME_FORMAT = new SimpleDateFormat("HH:mm", Locale.US);

        static {
            TIME_FORMAT.setTimeZone(getGmtTimeZone());
        }

        private boolean m_enabled;
        private ScheduledDay m_day;
        private Date m_start;
        private Date m_stop;

        public WorkingHours() {
            Calendar calendar = Calendar.getInstance(getGmtTimeZone());
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
            return formatTime(m_start);
        }

        public String getStopTime() {
            return formatTime(m_stop);
        }

        /**
         * @return UTC time
         */
        public Date getStart() {
            return m_start;
        }

        /**
         * @param start UTC time
         */
        public void setStart(Date start) {
            m_start = start;
        }

        public Date getStop() {
            return m_stop;
        }

        public void setStop(Date stop) {
            m_stop = stop;
        }

        /**
         * Formats time as 24 hours (00:00-23:59), GMT time.
         * 
         * @param date date to format - needs to be GMT time zon
         * @return formatted time string
         */
        private static String formatTime(Date date) {
            return TIME_FORMAT.format(date);
        }

        private static TimeZone getGmtTimeZone() {
            return TimeZone.getTimeZone("GMT");
        }
    }
}
