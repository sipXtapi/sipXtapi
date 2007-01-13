/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import java.util.Locale;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;

public class SettingPermission extends Permission {
    private MessageSource m_messageSource;

    private String m_name;

    private String m_labelKey;

    private String m_descriptionKey;

    public SettingPermission(Setting setting) {
        m_name = setting.getName();
        setLabel(setting.getLabel());
        setDescription(setting.getDescription());
        setDefaultValue(isEnabled(setting.getValue()));
        m_messageSource = setting.getMessageSource();
        m_labelKey = setting.getLabelKey();
        m_descriptionKey = setting.getDescriptionKey();
    }

    public boolean isBuiltIn() {
        return true;
    }

    public Object getPrimaryKey() {
        return m_name;
    }

    public String getName() {
        return m_name;
    }

    public String getLabel(Locale locale) {
        if (m_messageSource == null) {
            return super.getLabel(locale);
        }
        return m_messageSource.getMessage(m_labelKey, null, getLabel(), locale);
    }

    public String getDescription(Locale locale) {
        if (m_messageSource == null) {
            return super.getDescription(locale);
        }
        return m_messageSource.getMessage(m_descriptionKey, null, getDescription(), locale);
    }
}
