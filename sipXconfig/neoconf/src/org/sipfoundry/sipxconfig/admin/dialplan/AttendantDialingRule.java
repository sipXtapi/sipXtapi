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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

public class AttendantDialingRule extends DialingRule {
    private static final String SYSTEM_NAME_PREFIX = "aa_";

    private ScheduledAttendant m_afterHoursAttendant;
    private Holiday m_holidayAttendant;
    private WorkingTime m_workingTimeAttendant;
    private String m_attendantAliases;
    private String m_extension;

    public void appendToGenerationRules(List rules) {
        if (!isEnabled()) {
            return;
        }
        String[] aliases = InternalRule.getAttendantAliasesAsArray(m_attendantAliases);
        DialingRule attendantRule = new MappingRule.Operator(getName(), getDescription(),
                getSystemName(), m_extension, aliases);
        rules.add(attendantRule);
    }

    /**
     * This is the name passed to the mediaserver cgi to locate the correct auto attendant. It's
     * invalid until saved to database.
     */
    public String getSystemName() {
        return SYSTEM_NAME_PREFIX + getId();
    }

    public String[] getPatterns() {
        return null;
    }

    public Transform[] getTransforms() {
        return null;
    }

    public DialingRuleType getType() {
        return DialingRuleType.ATTENDANT;
    }

    public boolean isInternal() {
        return true;
    }

    public ScheduledAttendant getAfterHoursAttendant() {
        return m_afterHoursAttendant;
    }

    public void setAfterHoursAttendant(ScheduledAttendant afterHoursAttendant) {
        m_afterHoursAttendant = afterHoursAttendant;
    }

    public Holiday getHolidayAttendant() {
        return m_holidayAttendant;
    }

    public void setHolidayAttendant(Holiday holidayAttendant) {
        m_holidayAttendant = holidayAttendant;
    }

    public WorkingTime getWorkingTimeAttendant() {
        return m_workingTimeAttendant;
    }

    public void setWorkingTimeAttendant(WorkingTime workingTimeAttendant) {
        m_workingTimeAttendant = workingTimeAttendant;
    }

    public String getAttendantAliases() {
        return m_attendantAliases;
    }

    public void setAttendantAliases(String attendantAliases) {
        m_attendantAliases = attendantAliases;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }
}
