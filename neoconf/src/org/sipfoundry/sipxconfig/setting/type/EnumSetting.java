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

import org.apache.commons.collections.map.LinkedMap;
import org.apache.commons.lang.ObjectUtils;

public class EnumSetting implements SettingType {
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
}
