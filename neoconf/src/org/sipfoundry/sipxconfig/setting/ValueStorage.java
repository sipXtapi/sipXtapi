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
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * This represents a link table between several business objects and SettingValues
 * It serves no business purpose but gets around a hibernate and RDBMS mismatch
 */
public class ValueStorage implements Map, Serializable {
    
    private static final long serialVersionUID = 1L;      

    private int m_id = PhoneContext.UNSAVED_ID;
    
    private Map m_delegate = new HashMap();
    
    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }
 
    public Map getValues() {
        return m_delegate;
    }
    
    public void setValues(Map delegate) {
        m_delegate = delegate;
    }
    
    public void clear() {
        m_delegate.clear();
    }

    public boolean containsKey(Object key) {
        return m_delegate.containsKey(key);
    }

    public boolean containsValue(Object value) {
        return m_delegate.containsValue(value);
    }

    public Set entrySet() {
        return m_delegate.entrySet();
    }

    public Object get(Object key) {
        return m_delegate.get(key);
    }

    public boolean isEmpty() {
        return m_delegate.isEmpty();
    }

    public Set keySet() {
        return m_delegate.keySet();
    }

    public Object put(Object key_, Object value) {
        SettingValue settingValue = (SettingValue) value;
        return m_delegate.put(settingValue.getPath(), settingValue);
    }

    public void putAll(Map t) {
        m_delegate.putAll(t);
    }

    public Object remove(Object key) {
        return m_delegate.remove(key);
    }

    public int size() {
        return m_delegate.size();
    }

    public Collection values() {
        return m_delegate.values();
    }
}
