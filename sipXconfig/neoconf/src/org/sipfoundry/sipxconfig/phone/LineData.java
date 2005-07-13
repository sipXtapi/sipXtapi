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
package org.sipfoundry.sipxconfig.phone;

import java.util.Set;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Association between Users and their assigned phones.
 */
public class LineData extends BeanWithGroups {

    private User m_user;
    
    private PhoneData m_endpoint;

    private int m_position;
    
    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getDisplayLabel() {
        return m_user.getDisplayId();
    }

    public PhoneData getPhoneData() {
        return m_endpoint;
    }

    public void setPhoneData(PhoneData endpoint) {
        m_endpoint = endpoint;
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }
    
    protected Setting decorate(Group implicitRootTag, Set groups_, Setting settings) {
        // Use phone groups until we can justify lines
        // having their own groups and work out a reasonable UI
        Set phoneGroups = getPhoneData().getGroups();
        Setting decorated = super.decorate(implicitRootTag, phoneGroups, settings);
        
        return decorated;        
    }
    
}
