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
package org.sipfoundry.sipxconfig.phone;

import java.io.File;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public abstract class AbstractLine implements Line, PrimaryKeySource {

    private LineData m_meta;

    private Phone m_phone;

    private Setting m_settings;

    private String m_modelFilename;

    public PhoneContext getPhoneContext() {
        return getPhone().getPhoneContext();
    }

    public void setModelFilename(String modelFilename) {
        m_modelFilename = modelFilename;
    }

    public String getModelFilename() {
        return m_modelFilename;
    }

    public void setLineData(LineData meta) {
        m_meta = meta;
        m_meta.setPhoneData(getPhone().getPhoneData());
    }

    public LineData getLineData() {
        return m_meta;
    }

    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public Setting getSettings() {
        if (m_settings == null) {
            Setting settings = getSettingModel();
            setDefaults(settings);
            decorateSettings(settings);
        }

        return m_settings;
    }

    protected abstract void setDefaults(Setting settings);

    public Setting getSettingModel() {
        File modelDefsFile = new File(getPhoneContext().getSystemDirectory() + '/'
                + m_modelFilename);
        return new XmlModelBuilder().buildModel(modelDefsFile).copy();
    }

    protected void decorateSettings(Setting settings) {
        m_settings = settings;
        ValueStorage valueStorage = m_meta.getValueStorage();
        if (valueStorage == null) {
            valueStorage = new ValueStorage();
            m_meta.setValueStorage(valueStorage);
        }

        Folder folder = m_meta.getFolder();
        if (folder != null) {
            m_settings = (SettingGroup) folder.decorate(m_settings);
        }

        m_settings = (SettingGroup) valueStorage.decorate(m_settings);
    }

    public int getPosition() {
        return m_meta.getPosition();
    }

    public void setPosition(int position) {
        m_meta.setPosition(position);
    }

    public Object getPrimaryKey() {
        return m_meta.getId();
    }

    public String getUri() {
        String uri = null;
        User user = m_meta.getUser();
        if (user != null) {
            StringBuffer sb = new StringBuffer();
            sb.append("sip:").append(user.getDisplayId());
            sb.append('@').append(getPhoneContext().getDnsDomain());

            String displayName = user.getDisplayName();
            if (displayName != null) {
                sb.insert(0, "\"" + displayName + "\"<").append(">");
            }
            uri = sb.toString();
        }

        return uri;
    }
}