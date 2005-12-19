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
package org.sipfoundry.sipxconfig.common;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Calendar;
import java.util.Date;

import org.hibernate.type.TimestampType;

/**
 * Standard Hibernate assume that type is kept in local time zone and will normalize it to UTC.
 * Use this type if your type is already normalized to UTC.
 */
public class UtcTimestampType extends TimestampType {
    private Calendar m_local;

    private Calendar getLocalCalendar() {
        if (m_local == null) {
            m_local = Calendar.getInstance();
        }
        return m_local;
    }

    /**
     * value has been already converted to what Java thought was UTC value, we need to revert the
     * results of that conversion
     */
    public void set(PreparedStatement st, Object value, int index) throws SQLException {
        Date date = (Date) value;
        System.err.println("before: " + date);
        Calendar local = getLocalCalendar();
        local.setTime(date);
        int offset = local.get(Calendar.ZONE_OFFSET);
        local.add(Calendar.MILLISECOND, -offset);
        Date localTime = local.getTime();
        System.err.println("after: " + localTime);
        super.set(st, localTime, index);
    }

    /**
     * result of this function will be converted to local value, we need to add now offset that
     * will be removed by that conversion
     */
    public Object get(ResultSet rs, String name) throws SQLException {
        Date value = (Date) super.get(rs, name);
        Calendar local = getLocalCalendar();
        local.setTime(value);
        int offset = local.get(Calendar.ZONE_OFFSET);
        local.add(Calendar.MILLISECOND, offset);
        return local.getTime();
    }
}
