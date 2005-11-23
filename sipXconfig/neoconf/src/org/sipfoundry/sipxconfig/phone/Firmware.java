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
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * Describing the files required to track and manage a vendor's firmware files
 */
public class Firmware extends BeanWithSettings {
    private String m_name;
    private String m_description;
    private String m_deliveryId;
    private FirmwareManufacturer m_manufacturer;
    private String m_modelId;
    private String m_versionId;
    private ModelFilesContext m_modelFilesContext;
    private String m_uploadDirectory;

    public Setting getSettingModel() {
        Setting model = super.getSettingModel();
        if (model == null) {
            // TODO: Details
            model = m_modelFilesContext.loadModelFile("firmware.xml", m_manufacturer
                    .getManufacturerId(), null);
            setSettingModel(model);
        }
        return model;
    }

    protected void defaultSettings() {
        getSettings().acceptVisitor(new FirmwareDirectorySetter());
    }

    private class FirmwareDirectorySetter extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_uploadDirectory);
            }
        }
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public FirmwareManufacturer getManufacturer() {
        return m_manufacturer;
    }

    public void setManufacturer(FirmwareManufacturer manufacturer) {
        m_manufacturer = manufacturer;
    }

    public String getModelId() {
        return m_modelId;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    public String getVersionId() {
        return m_versionId;
    }

    public void setVersionId(String versionId) {
        m_versionId = versionId;
    }

    public String getDeliveryId() {
        return m_deliveryId;
    }

    public void setDeliveryId(String deliveryId) {
        m_deliveryId = deliveryId;
    }

    public ModelFilesContext getModelFilesContext() {
        return m_modelFilesContext;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    public String getUploadDirectory() {
        return m_uploadDirectory;
    }

    public void setUploadDirectory(String uploadDirectory) {
        m_uploadDirectory = uploadDirectory;
    }
}
