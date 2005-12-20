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

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class Holiday extends ScheduledAttendant {
    private List m_dates = new ArrayList();

    public void addDay(Date day) {
        m_dates.add(day);
    }

    public void removeDay(Date day) {
        m_dates.remove(day);
    }

    public List getDates() {
        return m_dates;
    }

    public void setDates(List dates) {
        m_dates = dates;
    }

    public void removeDay(int indexToDelete) {
        m_dates.remove(indexToDelete);
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     * 
     * @param i day index
     */
    public void setDay(int i, Date holidayDay) {
        if (i >= m_dates.size()) {
            m_dates.add(holidayDay);
        } else {
            m_dates.set(i, holidayDay);
        }
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     * 
     * @param i day index
     */
    public Date getDay(int i) {
        if (i < m_dates.size()) {
            return (Date) m_dates.get(i);
        }
        Date date = new Date();
        m_dates.add(date);
        return date;
    }

    /**
     * Remove all days that have indexes bigger that the one that is just passed.
     * 
     * @param maxDayIndex the index of the last objects retained in the list
     */
    public void chop(int maxDayIndex) {
        m_dates.subList(maxDayIndex + 1, m_dates.size()).clear();
    }
}
