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
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.map.LinkedMap;

/**
 * Meta information about a group of settings, can contain nested
 * SettingModels
 */
public class SettingModel extends SettingMeta {
    
    private Map m_delegate = new LinkedMap();
    
    /**
     * M A P  I M P L E M E N T A T I O N
     */
    public int size() {
        return m_delegate.size();
    }

    public void clear() {
        m_delegate.clear();
    }

    public boolean isEmpty() {
        return m_delegate.isEmpty();
    }

    public boolean containsKey(Object key) {
        return m_delegate.containsKey(key);
    }

    public boolean containsValue(Object value) {
        return m_delegate.containsValue(value);
    }

    public Collection values() {
        return m_delegate.values();
    }

    public void putAll(Map t) {
        m_delegate.putAll(t);
    }

    public Set entrySet() {
        return m_delegate.entrySet();
    }

    public Set keySet() {
        return m_delegate.keySet();
    }

    public Object get(Object key) {
        return m_delegate.get(key);
    }

    public Object remove(Object key) {
        return m_delegate.remove(key);
    }

    public Object put(Object key, Object value) {
        return m_delegate.put(key, value);
    }    
}
