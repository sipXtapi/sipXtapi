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

import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Uses pairs of option->label as a backend for Tapestry implementation
 */
public class NamedValuesSelectionModel implements IPropertySelectionModel {

    private final Object[] m_options;
    private final String[] m_labels;

    /**
     * @map if instance of commons LinkedMap, use it directly, otherwise copy into a order-aware
     *      map
     */
    public NamedValuesSelectionModel(Map map) {
        final int size = map.size();
        m_options = new Object[size];
        m_labels = new String[size];
        Iterator entries = map.entrySet().iterator();
        for (int i = 0; i < size; i++) {
            Map.Entry entry = (Entry) entries.next();
            m_options[i] = entry.getKey();
            m_labels[i] = (String) entry.getValue();
        }
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        return m_options[index];
    }

    public String getLabel(int index) {
        return m_labels[index];
    }

    public String getValue(int index) {
        return Integer.toString(index);
    }

    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return getOption(index);
    }
}
