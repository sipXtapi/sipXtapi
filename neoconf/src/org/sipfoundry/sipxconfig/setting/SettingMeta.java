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

import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.Set;


/**
 * Meta Information about a Setting.
 */
public class SettingMeta implements Map {

    private String m_label;

    private String m_type;

    private String m_name;

    private Setting m_setting;

    private String m_defaultValue;

    public void addMeta(SettingMeta meta) {
        put(meta.getName(), meta);
    }
    
    public SettingMeta getMeta(String name) {
        return (SettingMeta) get(name);
    }

    public String getDefaultValue() {
        return m_defaultValue;
    }

    public void setDefaultValue(String defaultValue) {
        m_defaultValue = defaultValue;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public Setting getSetting() {
        return m_setting;
    }

    public void setSetting(Setting setting) {
        m_setting = setting;
    }

    public String getType() {
        return m_type;
    }

    public void setType(String type) {
        m_type = type;
    }

    /**
     * M A P  I M P L E M E N T A T I O N
     */

    public int size() {
        return 0;
    }

    public void clear() {
    }

    public boolean isEmpty() {
        return true;
    }

    public boolean containsKey(Object key_) {
        return false;
    }

    public boolean containsValue(Object value_) {
        return false;
    }

    public Collection values() {
        return Collections.EMPTY_LIST;
    }

    public void putAll(Map t_) {
    }

    public Set entrySet() {
        return Collections.EMPTY_SET;
    }

    public Set keySet() {
        return Collections.EMPTY_SET;
    }

    public Object get(Object key_) {
        return null;
    }

    public Object remove(Object key_) {
        return null;
    }

    public Object put(Object key_, Object value_) {
        return null;
    }
}
