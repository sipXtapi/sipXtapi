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
        if (!m_dates.contains(day)) {
            m_dates.add(day);
        }
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
}
