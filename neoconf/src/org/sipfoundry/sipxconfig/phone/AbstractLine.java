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

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

public abstract class AbstractLine implements Line, PrimaryKeySource {

    private LineData m_meta;

    private Phone m_phone;

    private Setting m_settings;
    
    /** BEAN ACCESS ONLY */
    public AbstractLine() {        
    }
    
    public AbstractLine(Phone phone, LineData meta) {
        setPhone(phone);
        setLineData(meta);        
    }

    public void setLineData(LineData meta) {
        m_meta = meta;
        m_meta.setPhoneData(getPhone().getPhoneData());
    }

    public LineData getLineData() {
        return m_meta;
    }

    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public Setting getSettings() {
        if (m_settings == null) {

            m_settings = getSettingModel();
            ValueStorage valueStorage = m_meta.getValueStorage();
            if (valueStorage == null) {
                valueStorage = new ValueStorage();
                m_meta.setValueStorage(valueStorage);
            }

            Folder folder = m_meta.getFolder();
            if (folder != null) {
                m_settings = (SettingGroup) folder.decorate(m_settings);
            }

            m_settings = (SettingGroup) valueStorage.decorate(m_settings);
        }

        return m_settings;
    }
    
    public int getPosition() {
        return m_meta.getPosition();
    }
    
    public void setPosition(int position) {
        m_meta.setPosition(position);
    }

    public Object getPrimaryKey() {
        return m_meta.getId();
    }
}