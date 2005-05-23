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
import java.util.Date;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class DailyBackupSchedule extends BeanWithId {

    public static final DateFormat TIME_OF_DAY_FORMAT = DateFormat.getTimeInstance(DateFormat.SHORT);

    private boolean m_enabled;
        
    private Date m_timeOfDay = new Timestamp(0);

    private ScheduledDay m_day = ScheduledDay.EVERYDAY;

    private BackupPlan m_backupPlan;
    
    static {
        TIME_OF_DAY_FORMAT.setTimeZone(TimeZone.getTimeZone("GMT"));
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
}
