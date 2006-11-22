/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Property selection model for Java 1.5 enums
 */
public class NewEnumPropertySelectionModel<E extends Enum<E>> implements IPropertySelectionModel {

    private E[] m_options;

    public void setOptions(E[] options) {
        m_options = options;
    }

    public void setEnumType(Class<E> elementType) {
        m_options = elementType.getEnumConstants();
    }

    public int getOptionCount() {
        return m_options.length;
    }

    public Object getOption(int index) {
        return m_options[index];
    }

    public String getLabel(int index) {
        return m_options[index].name();
    }

    public String getValue(int index) {
        return new Integer(index).toString();
    }

    public Object translateValue(String value) {
        int i = Integer.parseInt(value);
        return m_options[i];
    }
}
