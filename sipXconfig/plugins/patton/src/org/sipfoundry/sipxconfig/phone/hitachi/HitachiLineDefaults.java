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
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class HitachiLineDefaults {
    private Line m_line;

    public HitachiLineDefaults(Line line) {
        m_line = line;
    }
    
    @SettingEntry(path = "SIP/Displayname")
    public String getDisplayName() {
        return m_line.getDisplayLabel();
    }

    @SettingEntry(path = "SIP/Phone_Number")
    public String getPhoneNumber() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        // FIXME: we need a method that returns a numerical alias for the user
        return user.getUserName();        
    }
        
    @SettingEntry(path = "SIP/User_ID")
    public String getUserName() {
        m_line.getDisplayLabel();
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getUserName();        
    }
        
    @SettingEntry(path = "SIP/User_Password")
    public String getSipPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getSipPassword();        
    }
}
