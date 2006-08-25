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
import java.util.Collections;

/**
 * Abstract setting with model (as in model/view/controller model)
 */
public class SettingImpl extends AbstractSetting implements Setting, Cloneable {
    private SettingModel2 m_model;

    /**
     * bean access only, must set name before valid object
     */
    public SettingImpl() {
    }

    public SettingImpl(String name) {
        setName(name);
    }

    @Override
    public String getProfileName() {
        String profileName = super.getProfileName();
        if (m_model == null) {
            return profileName;
        }
        SettingValue2 modelProfileName = m_model.getProfileName(this);
        if (modelProfileName == null) {
            return profileName;
        }
        return modelProfileName.getValue();
    }

    @Override
    public String getValue() {
        if (m_model != null) {
            SettingValue2 value = m_model.getSettingValue(this);
            if (value != null) {
                return value.getValue();
            }
        }
        return super.getValue();
    }

    @Override
    public void setValue(String value) {
        if (m_model != null) {
            m_model.setSettingValue(this, value);
        } else {
            super.setValue(value);
        }
    }

    public String getDefaultValue() {
        if (m_model != null) {
            SettingValue2 value = m_model.getDefaultSettingValue(this);
            if (value != null) {
                return value.getValue();
            }
        }
        return super.getValue();
    }

    public Collection<Setting> getValues() {
        return Collections.emptyList();
    }

    public void setModel(SettingModel2 model) {
        m_model = model;
    }

    public SettingModel2 getModel() {
        return m_model;
    }
}
