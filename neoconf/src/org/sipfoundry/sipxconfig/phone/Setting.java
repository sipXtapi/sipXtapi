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

import java.io.Serializable;
import java.util.Collections;
import java.util.Map;


/**
 * Represent one setting in a set of settings. Composite base class to SettingSet
 */
public class Setting implements Serializable {

    private static final long serialVersionUID = 1L;

    private String m_name;

    private Object m_value;
    
    private Object m_default;
    
    private int m_id = PhoneDao.UNSAVED_ID;
    
    private SettingSet m_parent;

    public Setting() {
    }

    public Setting(String name) {
        this(name, null);
    }

    public Setting(String name, Object value) {
        m_name = name;
        m_value = value;
    }

    public Object getDefault() {
        return m_default;
    }
    
    public void setDefault(Object default1) {
        m_default = default1;
    }
    
    /**
     * @return null only if subclass is SettingSet and it's the root setting
     */
    public SettingSet getParent() {
        return m_parent;
    }

    /**
     * Called internally when a setting is added to its parent
     */
    public void setParent(SettingSet parent) {
        m_parent = parent;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public Object getValue() {
        return m_value;
    }
        
    /**
     * @return string representation of value
     */
    public String getString() {
        return (m_value != null ? m_value.toString() : null);
    }

    /**
     * Called only if the value is supposed to be a string
     * @param string
     */
    public void setString(String string) {
        m_value = string;
    }

    public void setValue(Object value) {
        m_value = value;
    }
    
    public Map getSettings() {
        return Collections.EMPTY_MAP;
    }
}
