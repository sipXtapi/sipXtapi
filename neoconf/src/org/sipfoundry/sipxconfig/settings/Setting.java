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
package org.sipfoundry.sipxconfig.settings;

import java.util.Collections;
import java.util.Map;


public class Setting {

    private String m_name;

    private Object m_value;
    
    private int m_id = -1;
    
    private SettingSet m_parent;

    public Setting() {
    }

    public Setting(String name) {
        m_name = name;
    }

    public Setting(String name, Object value) {
        m_name = name;
        m_value = value;
    }

    /**
     * @return Returns the parent.
     */
    public SettingSet getParent() {
        return m_parent;
    }
    /**
     * @param parent The parent to set.
     */
    public void setParent(SettingSet parent) {
        m_parent = parent;
    }
    /**
     * @return Returns the id.
     */
    public int getId() {
        return m_id;
    }

    /**
     * @param id The id to set.
     */
    public void setId(int id) {
        m_id = id;
    }

    /**
     * @return Returns the name.
     */
    public String getName() {
        return m_name;
    }

    /**
     * @param name The name to set.
     */
    public void setName(String name) {
        m_name = name;
    }

    /**
     * @return Returns the value.
     */
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

    /**
     * @param value The value to set.
     */
    public void setValue(Object value) {
        m_value = value;
    }
    
    /* to fulfill composite design pattern, but unclear there's a need at this
     * point.  TODO : Remove if not needed
    public void addSetting(Setting settingTemp) {
        throw new IllegalArgumentException("Cannot add setting to leaf setting");
    }

    public Setting getSetting(String settingNameTemp) {
        throw new IllegalArgumentException("Cannot get settings on leaf setting");
    }
    */

    public Map getSettings() {
        return Collections.EMPTY_MAP;
    }
}
