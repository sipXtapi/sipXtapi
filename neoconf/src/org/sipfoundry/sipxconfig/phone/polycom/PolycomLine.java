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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;


/**
 * Polycom business functions for line meta setting
 */
public class PolycomLine {
    
    public static final String REGISTRATION = "reg";
    
    public static final String SERVER = "server";
    
    public static final String ADDRESS = "address";
    
    public static final String DISPLAY_NAME = "displayName";
    
    public static final String FIRST = "1";
    
    private PolycomPhone m_phone;
    
    private Line m_line;
    
    private Setting m_settings;
    
    public PolycomLine(PolycomPhone phone, Line line) {
        m_phone = phone;
        m_line = line;
        m_settings = m_phone.getLineModel().copy();
        setDefaults();
    }
    
    private void setDefaults() {
        User u = m_line.getUser();
        if (u != null) {
            String domainName = m_phone.getCoreContext().loadRootOrganization().getDnsDomain();
            Setting reg = m_settings.getSetting(REGISTRATION);
            reg.getSetting(DISPLAY_NAME).setValue(u.getDisplayId());
            reg.getSetting("auth.userId").setValue(u.getDisplayId());
            reg.getSetting(SERVER).getSetting(FIRST).getSetting(ADDRESS).setValue(domainName);
        }

        // See pg. 125 Admin Guide/16 June 2004
        if (m_line.getPosition() == 0) {
            m_settings.getSetting("msg.mwi").getSetting("callBackMode").setValue("registration");
        }        
    }
    
    public Setting getSettings() {
        return m_settings;
    }
}
