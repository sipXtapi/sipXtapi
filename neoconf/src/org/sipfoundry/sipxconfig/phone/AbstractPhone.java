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

import java.util.ArrayList;
import java.util.List;

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
    
    private List m_lines = new ArrayList();
    
    /** BEAN ACCESS ONLY */
    public AbstractPhone() {        
    }
    
    public AbstractPhone(PhoneMetaData meta) {
        setPhoneMetaData(meta);
    }
    
    public void setPhoneMetaData(PhoneMetaData meta) {
        m_meta = meta;
        m_lines.clear();
        List lineMeta = meta.getLines();
        for (int i = 0; lineMeta != null && i < lineMeta.size(); i++) {
            Line line = createLine();
            line.setLineMetaData((LineMetaData) lineMeta.get(i));
            m_lines.add(line);
        }
    }
    
    public int getLineCount() {
        return m_meta.getLines().size();
    }
    
    public Line getLine(int position) {        
        return (Line) m_lines.get(position);
    }
    
    public void addLine(Line line) {
        m_meta.addLine(line.getLineMetaData());
        m_lines.add(line);
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
