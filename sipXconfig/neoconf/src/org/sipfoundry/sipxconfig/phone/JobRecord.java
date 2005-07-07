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
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.Date;

import org.sipfoundry.sipxconfig.common.CoreContext;

/**
 * Represent a job record in 
 */
public class JobRecord implements Serializable {

    public static final int TYPE_PROJECTION = 0;

    public static final int TYPE_DEVICE_RESTART = 1;

    public static final char STATUS_UNKNOWN = 0;

    public static final char STATUS_STARTED = 'S';

    public static final char STATUS_FAILED = 'F';

    public static final char STATUS_COMPLETE = 'C';

    private static final DateFormat START_TIME_FORMAT = DateFormat.getDateTimeInstance(
            DateFormat.MEDIUM, DateFormat.MEDIUM);

    private Integer m_id = CoreContext.UNSAVED_ID;

    private int m_type;

    private char m_status = STATUS_UNKNOWN;

    private String m_startTimeString;

    private String m_details;

    private String m_progress;

    private String m_exceptionMessage;
    
    private Phone[] m_phones;    
    
    /**
     * Phone that job will operate on
     */
    public void setPhones(Phone[] phones) {
        m_phones = phones;
    }
    
    public Phone[] getPhones() {
        return m_phones;
    }    

    public String getDetails() {
        return m_details;
    }

    public void setDetails(String details) {
        this.m_details = details;
    }

    public String getExceptionMessage() {
        return m_exceptionMessage;
    }

    public void setExceptionMessage(String exceptionMessage) {
        m_exceptionMessage = exceptionMessage;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public String getProgress() {
        return m_progress;
    }

    public void setProgress(String progress) {
        m_progress = progress;
    }

    public Date getStartTime() {
        Date startTime = null;
        if (START_TIME_FORMAT != null) {
            try {
                return START_TIME_FORMAT.parse(m_startTimeString);
            } catch (ParseException e) {
                throw new RuntimeException("Bad job start time format in database: "
                        + m_startTimeString);
            }
        }
        return startTime;
    }

    public void setStartTime(Date startTime) {
        m_startTimeString = START_TIME_FORMAT.format(startTime);
    }

    /**
     * Only here to be backward compatible with database that stores this as string, 
     * Use setStartTime start time instead
     */
    public void setStartTimeString(String startTime) {
        m_startTimeString = startTime;
    }

    /**
     * Only here to be backward compatible with database that stores this as string, 
     * Use getStartTime start time instead
     */
    public String getStartTimeString() {
        return m_startTimeString;
    }

    public char getStatus() {
        return m_status;
    }

    public void setStatus(char status) {
        m_status = status;
        if (m_status == STATUS_STARTED) {
            setStartTime(new Date());
        }
    }

    public int getType() {
        return m_type;
    }

    public void setType(int type) {
        m_type = type;
    }
}
