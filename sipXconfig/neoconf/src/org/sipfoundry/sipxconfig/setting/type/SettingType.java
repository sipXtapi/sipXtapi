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
package org.sipfoundry.sipxconfig.setting.type;

public interface SettingType {
    String getName();

    boolean isRequired();

    /**
     * Converts between internal representation of the value (usually String) and external
     * representation determined by this type.
     * 
     * @param value internal representation of the setting value
     * @return the same value coerced to typed value Integer, Boolean etc.
     */
    Object convertToTypedValue(Object value);
    
    String convertToStringValue(Object value);
    
    /**
     * Return human representations of value, not internal reprentations of value,
     * not always the same, for example boolean or list types 
     */
    String getLabel(Object value);

    void setId(String id);
}
