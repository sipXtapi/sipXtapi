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

public class StringSetting extends AbstractSettingType {
    public static final StringSetting DEFAULT = new StringSetting();

    private static final int DEFAULT_MAX_LEN = 256;

    private boolean m_required;

    private int m_maxLen = DEFAULT_MAX_LEN;

    private String m_pattern;

    private boolean m_password;

    public int getMaxLen() {
        return m_maxLen;
    }

    public void setMaxLen(int maxLen) {
        m_maxLen = maxLen;
    }

    public String getPattern() {
        return m_pattern;
    }

    public void setPattern(String pattern) {
        m_pattern = pattern;
    }

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }

    public boolean isPassword() {
        return m_password;
    }

    public void setPassword(boolean password) {
        m_password = password;
    }

    public String getName() {
        return "string";
    }

    public Object convertToTypedValue(Object value) {
        return value;
    }

    public String convertToStringValue(Object value) {
        return (String) value;
    }

    public String getLabel(Object value) {
        if (isPassword()) {
            return null;
        }
        return convertToStringValue(value);
    }
}
