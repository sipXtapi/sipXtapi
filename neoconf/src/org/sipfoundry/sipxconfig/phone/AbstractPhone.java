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

/**
 * Implements some of the more menial methods of Phone interface 
 */
public abstract class AbstractPhone implements Phone {

    private PhoneMetaData m_meta;
    
    private Setting m_settings;
    
    /** BEAN ACCESS ONLY */
    public AbstractPhone() {        
    }
    
    public AbstractPhone(PhoneMetaData meta) {
        setPhoneMetaData(meta);
        meta.setPhone(this);
    }
    
    public void setPhoneMetaData(PhoneMetaData meta) {
        m_meta = meta;
    }
    
    public int getLineCount() {
        return m_meta.getLines().size();
    }
    
    public Line getLine(int position) {
        return ((LineMetaData) m_meta.getLines().get(position)).getLine();
    }
    
    public void addLine(Line line) {
        m_meta.addLine(line.getLineMetaData());
    }
    
    public PhoneMetaData getPhoneMetaData() {
        return m_meta;
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
