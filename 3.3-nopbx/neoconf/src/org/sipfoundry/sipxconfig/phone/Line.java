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

import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

public class Line extends BeanWithGroups implements DataCollectionItem {

    private Phone m_phone;
    
    private User m_user;
    
    private int m_position;
    
    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getDisplayLabel() {
        LineSettings settings = (LineSettings) getAdapter(LineSettings.class);
        return (settings != null ? settings.getUserId() : null);
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }
    
    public Setting getSettingModel() {
        Setting settingModel = super.getSettingModel();
        if (settingModel == null) {
            settingModel = getPhone().loadModelFile("line.xml");
            setSettingModel(settingModel);
        }
        
        return settingModel;
    }
    
    public Set getGroups() {
        // Use phone groups until we can justify lines
        // having their own groups and work out a reasonable UI
        Set groups = getPhone().getGroups();
        
        return groups;
    }
    
    public PhoneContext getPhoneContext() {
        return getPhone().getPhoneContext();
    }

    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    protected void defaultSettings() {
        getPhone().defaultLineSettings(this);
    }
    
    protected void decorateSettings() {
        super.decorateSettings();
        
    }

    public Object getAdapter(Class interfac) {
        Object adapter = getPhone().getLineAdapter(this, interfac);
        return adapter;
    }
    
    public String getUri() {        
        LineSettings settings = (LineSettings) getAdapter(LineSettings.class);
        String uri = getPhoneContext().getPhoneDefaults().getUri(settings);
        return uri;
    }
}
