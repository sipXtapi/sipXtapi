/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;

public class LgNortelLineDefaults {

    private Line m_line;
    private DeviceDefaults m_defaults;

    public LgNortelLineDefaults(Line line, DeviceDefaults defaults) {
        m_line = line;
        m_defaults = defaults;
    }

    public String getUserName() {
        m_line.getDisplayLabel();
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getUserName();
    }

    public static LineInfo getLineInfo(LgNortelPhone phone, Line line) {
        // TODO Auto-generated method stub
        return null;
    }

    public static void setLineInfo(LgNortelPhone phone, Line line, LineInfo lineInfo) {
        // TODO Auto-generated method stub

    }
}
