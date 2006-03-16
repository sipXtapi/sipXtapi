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
import java.text.ParseException;
import java.util.Arrays;
import java.util.Date;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/** Restart media server and status server periodically due to memory leaks */
public class Whacker implements ApplicationListener {
    class WhackerTask extends TimerTask {
        public void run() {
            LOG.info("Restarting the media server");
            m_processContext.manageServices(Arrays.asList(SERVICES), SipxProcessContext.Command.RESTART);
        }
    }

    static final Process[] SERVICES = {
        Process.MEDIA_SERVER
    };
    private static final Log LOG = LogFactory.getLog(Whacker.class);

    private SipxProcessContext m_processContext;
    private boolean m_enabled = true;
    private String m_timeOfDay = DailyBackupSchedule.convertUsTime("3:42 AM"); // in the local timezone
    private String m_scheduledDay = "Sunday";
    private Timer m_timer;
    private boolean m_allowStaleDate;       // for testing only

    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getScheduledDay() {
        return m_scheduledDay;
    }

    public void setScheduledDay(String scheduledDay) {
        m_scheduledDay = scheduledDay;
    }

    public String getTimeOfDay() {
        return m_timeOfDay;
    }

    public void setTimeOfDay(String timeOfDay) {
        m_timeOfDay = timeOfDay;
    }
    
    // for testing only
    void setAllowStaleDate(boolean allowStaleDate) {
        m_allowStaleDate = allowStaleDate;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener interface are
        // automatically registered
        if (event instanceof ApplicationInitializedEvent) {
            resetTimer();
        }
    }

    private void resetTimer() {
        if (m_timer != null) {
            m_timer.cancel();
        }
        if (!isEnabled()) {
            LOG.info("Whacker is disabled");
            return;
        }
        m_timer = new Timer(false); // daemon, dies with main thread
        scheduleTask();
    }

    private void scheduleTask() {
        // Reuse the DailyBackupSchedule class to schedule this non-backup task.
        // Ideally we would do a little refactoring to make this clearer, but this code
        // is being written as a low-risk patch to 3.0, so that will have to wait.
        DailyBackupSchedule sched = new DailyBackupSchedule();
        sched.setEnabled(true);
        sched.setScheduledDay(getScheduledDayEnum());
        sched.setTimeOfDay(getTimeOfDayValue());
        sched.setAllowStaleDate(m_allowStaleDate);  // for testing only
        sched.schedule(m_timer, new WhackerTask());
        LOG.info("Whacker is scheduled: " + sched.getScheduledDay().getName() + ", " + getTimeOfDay());
    }

    /** Convert the ScheduledDayName string to a ScheduledDay and return it */
    ScheduledDay getScheduledDayEnum() {
        ScheduledDay day = ScheduledDay.getScheduledDay(getScheduledDay());
        if (day == null) {
            throw new RuntimeException("Whacker: unrecognized scheduled day: "
                    + getScheduledDay());
        }
        return day;
    }

    /**
     * Convert the time-of-day string to a Date, expressed in Universal Time because that is what
     * DailyBackupSchedule is expecting.
     */
    private Date getTimeOfDayValue() {
        DateFormat df = DateFormat.getTimeInstance(DateFormat.SHORT);
        df.setTimeZone(TimeZone.getTimeZone("GMT"));
        Date date = null;
        try {
            date = df.parse(getTimeOfDay());
        } catch (ParseException e) {
            throw new RuntimeException("Whacker: could not parse time of day: " + getTimeOfDay());
        }
        return date;
    }

}
