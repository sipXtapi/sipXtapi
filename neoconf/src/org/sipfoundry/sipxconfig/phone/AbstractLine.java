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

import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

public abstract class AbstractLine implements Line {

    private LineMetaData m_meta;

    private Phone m_phone;

    private Setting m_settings;
    
    /** BEAN ACCESS ONLY */
    public AbstractLine() {        
    }
    
    public AbstractLine(Phone phone) {
        setPhone(phone);
        LineMetaData meta = new LineMetaData();
        //meta.setLine(this);
        setLineMetaData(meta);        
    }

    public void setLineMetaData(LineMetaData meta) {
        m_meta = meta;
    }

    public LineMetaData getLineMetaData() {
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

}