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


public abstract class AbstractStorage implements Map, Serializable {
    
    private static final long serialVersionUID = 1L;      

    private Integer m_id = PhoneContext.UNSAVED_ID;
    
    private Map m_delegate = new HashMap();
    
    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
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

    public Object put(Object key, Object value) {
        return m_delegate.put(key, value);
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
    
    protected Map getDelegate() {
        return m_delegate;
    }
    
    protected void setDelegate(Map delegate) {
        m_delegate = delegate;
    }
}
