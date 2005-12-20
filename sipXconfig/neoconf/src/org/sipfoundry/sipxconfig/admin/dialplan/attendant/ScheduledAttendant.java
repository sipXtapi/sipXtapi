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

import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class ScheduledAttendant {
    private boolean m_enabled;

    private AutoAttendant m_attendant;

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public AutoAttendant getAttendant() {
        return m_attendant;
    }

    public void setAttendant(AutoAttendant attendant) {
        m_attendant = attendant;
    }

    /**
     * Check if the attendant in question is referenced by this schedule
     * 
     * @param attendant
     * @return true if any references have been found false otherwise
     */
    public boolean checkAttendant(AutoAttendant attendant) {
        if (m_attendant == null) {
            return false;
        }
        return m_attendant.equals(attendant);
    }
}
