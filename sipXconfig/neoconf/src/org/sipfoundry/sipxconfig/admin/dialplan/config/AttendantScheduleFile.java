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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public class AttendantScheduleFile extends XmlFile {
    public static final DateFormat DATE_FORMAT = new SimpleDateFormat("dd-MMM-yyyy", Locale.US);

    private Document m_document;

    public Document getDocument() {
        if (m_document == null) {
            Element root = XmlFile.FACTORY.createElement("organizationprefs");
            m_document = XmlFile.FACTORY.createDocument(root);
        }
        return m_document;
    }

    public ConfigFileType getType() {
        return ConfigFileType.ATTENDANT_SCHEDULE;
    }

    void generate(AttendantRule attendantRule) {
        Element root = getDocument().getRootElement();
        Element schedule = root.addElement("schedule");
        Holiday holidayAttendant = attendantRule.getHolidayAttendant();
        addHoliday(schedule, holidayAttendant);
        WorkingTime workingTimeAttendant = attendantRule.getWorkingTimeAttendant();
        addRegularHours(schedule, workingTimeAttendant);
        ScheduledAttendant afterHoursAttendant = attendantRule.getAfterHoursAttendant();
        addAttendant(schedule, afterHoursAttendant, "afterhours");
    }

    private void addHoliday(Element schedule, Holiday holiday) {
        Element holidays = addAttendant(schedule, holiday, "holidays");
        List dates = holiday.getDates();
        for (Iterator i = dates.iterator(); i.hasNext();) {
            Date date = (Date) i.next();
            holidays.addElement("date").setText(DATE_FORMAT.format(date));
        }
    }

    private void addRegularHours(Element schedule, WorkingTime workingTime) {
        Element regularHours = addAttendant(schedule, workingTime, "regularhours");
        WorkingHours[] workingHours = workingTime.getWorkingHours();
        for (int i = 0; i < workingHours.length; i++) {
            WorkingHours whs = workingHours[i];
            if (!whs.isEnabled()) {
                continue;
            }
            Element day = regularHours.addElement(whs.getDay().getName().toLowerCase());
            day.addElement("from").setText(whs.getStartTime());
            day.addElement("to").setText(whs.getStopTime());
        }
    }

    private Element addAttendant(Element parent, ScheduledAttendant sa, String name) {
        Element ah = parent.addElement(name);
        ah.addElement("filename").setText(sa.getAttendant().getSystemName());
        return ah;
    }
}
