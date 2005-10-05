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
package org.sipfoundry.sipxconfig.gateway;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingModel;

/**
 * Gateway
 */
public class Gateway extends BeanWithSettings implements NamedObject {
    private String m_name;

    private String m_address;

    private String m_description;

    private String m_beanId;

    private String m_modelId;

    private String m_serialNumber;

    private String m_tftpRoot;

    private PhoneModel m_model;

    public Gateway() {
    }

    public void generateProfiles(Writer writer_) throws IOException {
        // generic gateways does not support generating profiles
        throw new UnsupportedOperationException();
    }

    public void prepareSettings() {
        // do nothing for generic gateways - this method is called before generateProfiles is
        // called
        // use to adjust settings for generation
    }

    public void generateProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
    }

    public void removeProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
    }

    public Setting getSettingModel() {
        Setting model = super.getSettingModel();
        if (model instanceof SettingModel) {
            SettingModel settingModel = (SettingModel) model;
            return settingModel.getRealSetting();
        }
        return null;
    }

    /**
     * This is slightly unusual setter - used to initialed model variable TODO: replace with
     * proper hibernate mappings
     * 
     * @param modelId
     */
    public void setModelId(String modelId) {
        m_modelId = modelId;
        m_model = PhoneModel.getModel(m_beanId, modelId);
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

    public String getTftpRoot() {
        return m_tftpRoot;
    }

    public void setTftpRoot(String tftpRoot) {
        m_tftpRoot = tftpRoot;
    }

    public PhoneModel getModel() {
        return m_model;
    }

    public void setModel(PhoneModel model) {
        m_model = model;
    }

    public String getBeanId() {
        return m_beanId;
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public String getModelId() {
        return m_modelId;
    }

    protected void propagate() {
        prepareSettings();
        generateProfiles();
    }
}
