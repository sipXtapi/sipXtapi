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
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.map.LinkedMap;

/**
 * Meta information about a group of settings, can contain nested
 * SettingModels.  Order is preserved
 */
public class SettingModel extends SettingMeta {
    
    private LinkedMap m_delegate = new LinkedMap();

    /**
     * Create the top most model
     */
    public SettingModel() {
        super(null);
    }
    
    public SettingModel(String name) {
        super(name);
    }
    
    /**
     * generate a mutable, deep copy populated with the settings
     */
    public SettingModel populateCopy(SettingSet set) {
        SettingModel clone = (SettingModel) this.clone();
        clone.setSetting(set);
        // TODO: see why m_delegate.size() can return -1
        //clone.m_delegate = new LinkedMap(m_delegate.size());        
        clone.m_delegate = new LinkedMap(m_delegate.size());        
        Iterator values = m_delegate.values().iterator();
        while (values.hasNext()) {
            SettingMeta meta = (SettingMeta) values.next();
            meta.setSetting(set.getSetting(meta.getName()));
            clone.put(meta.getName(), meta.clone());
        }
        clone.m_delegate = (LinkedMap) m_delegate.clone();        

        return clone;
    }
    
    /**
     * the nth item that was added to this model
     */
    public SettingMeta getMeta(int index) {
        Object key = m_delegate.get(index);
        return (SettingMeta) (key != null ? m_delegate.get(key) : null);
    }

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
