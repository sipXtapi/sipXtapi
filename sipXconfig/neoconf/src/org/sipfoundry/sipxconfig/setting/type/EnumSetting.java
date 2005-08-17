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

import java.util.Collections;
import java.util.Map;

import org.apache.commons.beanutils.ConversionException;
import org.apache.commons.beanutils.Converter;
import org.apache.commons.beanutils.converters.IntegerConverter;
import org.apache.commons.collections.map.LinkedMap;
import org.apache.commons.lang.ObjectUtils;

public class EnumSetting implements SettingType {
    private static final Converter CONVERTER = new IntegerConverter();
    
    private Map m_enums = new LinkedMap();

    public EnumSetting() {
    }

    public String getName() {
        return "enum";
    }

    public void addEnum(String value, String label) {
        m_enums.put(value, ObjectUtils.defaultIfNull(label, value));
    }

    public Map getEnums() {
        return Collections.unmodifiableMap(m_enums);
    }
    
    public boolean isRequired() {
        return false;
    }

    /**
     * At the moment we do not know if enumeration values are integers or strings.
     * The naive implementation tries to coerce the value to integer, if that fails strings are used.
     */
    public Object convertToTypedValue(Object value) {
        try {
            return CONVERTER.convert(Integer.class, value);
            
        } catch (ConversionException e) {
            return value;
        }
    }
}
