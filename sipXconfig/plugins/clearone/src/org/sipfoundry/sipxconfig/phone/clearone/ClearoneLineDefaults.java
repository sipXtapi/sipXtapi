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
package org.sipfoundry.sipxconfig.phone.clearone;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class ClearoneLineDefaults {
    private Line m_line;

    public ClearoneLineDefaults(Line line) {
        m_line = line;
    }
    
    @SettingEntry(path = "USER_ACCOUNT/Displayname")
    public String getDisplayName() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getDisplayName();
        } 
        return m_line.getLineInfo().getUserId();
    }

    @SettingEntry(path = "USER_ACCOUNT/Phone_Number")
    public String getPhoneNumber() {
        // FIXME: we need a method that returns a numerical alias for the user
        return getUserName();
    }
        
    @SettingEntry(path = "USER_ACCOUNT/User_ID")
    public String getUserName() {
        m_line.getDisplayLabel();
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getUserName();        
    }
        
    @SettingEntry(path = "USER_ACCOUNT/User_Password")
    public String getSipPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getSipPassword();        
    }
}
