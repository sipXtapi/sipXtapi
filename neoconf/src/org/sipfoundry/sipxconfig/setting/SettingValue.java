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
package org.sipfoundry.sipxconfig.setting;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Represent just the setting value, absent of meta data, this is meant to be saved to the database
 */
public class SettingValue implements Serializable {

    private static final long serialVersionUID = 1L;

    private String m_path;

    private String m_value;
    
    private int m_id = PhoneContext.UNSAVED_ID;
    
    /** hibernate only */
    private SettingMap m_map;
    
    /** bean access only */
    public SettingValue() {        
    }
    
    public SettingValue(String path, String value) {
        m_path = path;
        m_value = value;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getPath() {
        return m_path;
    }

    public void setPath(String path) {
        m_path = path;
    }

    public String getValue() {
        return m_value;
    }
        
    public void setValue(String value) {
        m_value = value;
    }


    /** hibernate only */
    public void setSettingMap(SettingMap map) {
        m_map = map;
    }

    /** hibernate only */
    public SettingMap getSettingMap() {
        return m_map;
    }
}
