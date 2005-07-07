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
package org.sipfoundry.sipxconfig.components;

import java.util.Arrays;
import java.util.Collection;
import java.util.Map;

/**
 * Let's you set keys on map entries without losing your place in the iteration thru the map. This is
 * handy in tapestry when you iterate thru a list and your key values can change. 
 */
public class MutableKeyMapAdapter {

    private Map m_map;

    private Object[] m_entries;

    private Object m_currentEntry;

    /**
     * @param map order is preserved, so linked or tree maps supported.
     */
    public void setMap(Map map) {
        m_map = map;
        m_entries = map.keySet().toArray();
    }

    public Collection getEntries() {
        return Arrays.asList(m_entries);
    }

    public void setCurrentEntry(Object entry) {
        m_currentEntry = entry;
    }

    public Object getCurrentEntry() {
        return m_currentEntry;
    }

    public void setKey(Object key) {
        Object o = m_map.get(m_currentEntry);
        m_map.remove(m_currentEntry);
        m_map.put(key, o);
        m_currentEntry = key;
    }

    public Object getKey() {
        return m_currentEntry;
    }
    
    public Object getValue() {
        Object value = m_map.get(m_currentEntry);
        return value;
    }
    
    public void setValue(Object value) {
        m_map.put(m_currentEntry, value);
    }
}
