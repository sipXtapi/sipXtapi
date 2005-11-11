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

import java.sql.Timestamp;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class DailyBackupSchedule extends BeanWithId {
    
    public static final DateFormat GMT_TIME_OF_DAY_FORMAT = DateFormat
            .getTimeInstance(DateFormat.SHORT);

    public static final DateFormat LOCAL_TIME_OF_DAY_FORMAT = DateFormat
            .getTimeInstance(DateFormat.SHORT);

    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    private static final TimeZone GMT = TimeZone.getTimeZone("GMT");

    private static final long ONCE_A_DAY = 1000 * 60 * 60 * 24;
    
    private static final long ONCE_A_WEEK = ONCE_A_DAY * 7;

    private boolean m_enabled;

    private Date m_timeOfDay = new Timestamp(0);

    private ScheduledDay m_day = ScheduledDay.EVERYDAY;

    private BackupPlan m_backupPlan;
    
    static {
        // Storing dates in GMT keeps times consistent if timezones change
        // disadvantage, conversions of date outside this converter appear
        // wrong unless you live in grenwich
        GMT_TIME_OF_DAY_FORMAT.setTimeZone(GMT);        
    }

    public BackupPlan getBackupPlan() {
        return m_backupPlan;
    }

    public void setBackupPlan(BackupPlan backupPlan) {
        m_backupPlan = backupPlan;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public ScheduledDay getScheduledDay() {
        return m_day;
    }

    public void setScheduledDay(ScheduledDay day) {
        m_day = day;
    }

    public Date getTimeOfDay() {
        return m_timeOfDay;
    }

    public void setTimeOfDay(Date timeOfDay) {
        m_timeOfDay = timeOfDay;
    }
    
    Date getTimerDate() {
        
        // convert thru string because setting timezone just shift time and
        // we want effectively the same time.  
        String gmtTimeOfDay = GMT_TIME_OF_DAY_FORMAT.format(getTimeOfDay());
        try {
            Date localTimeOfDay = LOCAL_TIME_OF_DAY_FORMAT.parse(gmtTimeOfDay);
            Calendar localCal = Calendar.getInstance();
            localCal.setTime(localTimeOfDay);
            
            Calendar when = Calendar.getInstance();
            when.set(Calendar.HOUR_OF_DAY, localCal.get(Calendar.HOUR_OF_DAY));
            when.set(Calendar.MINUTE, localCal.get(Calendar.MINUTE));
            when.set(Calendar.SECOND, 0);
            when.set(Calendar.MILLISECOND, 0);
            
            if (m_day != ScheduledDay.EVERYDAY) {
                when.set(Calendar.DAY_OF_WEEK, getScheduledDay().getDayOfWeek());
            }
            
            return when.getTime();        
        } catch (ParseException e) {
            throw new RuntimeException(e);
        }        
    }
    
    long getTimerPeriod() {
        return (m_day == ScheduledDay.EVERYDAY ? ONCE_A_DAY : ONCE_A_WEEK);
    }

    public void schedule(Timer timer, TimerTask task) {
        if (!isEnabled()) {
            return;
        }

        Date date = getTimerDate();
        long period = getTimerPeriod();
        LOG.info("Setting timer for " + LOCAL_TIME_OF_DAY_FORMAT.format(date));
        timer.schedule(task, date, period);
    }
}
