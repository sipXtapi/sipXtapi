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
package org.sipfoundry.sipxconfig.upload;

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
    private FirmwareManufacturer m_manufacturer;
    private ModelFilesContext m_modelFilesContext;

    public Setting getSettingModel() {
        Setting model = super.getSettingModel();
        if (model == null) {
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
                String uploadDir = getManufacturer().getFileDelivery().getUploadDirectory();
                fileType.setDirectory(uploadDir);
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

    public ModelFilesContext getModelFilesContext() {
        return m_modelFilesContext;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }
    
    public void remove() {
        // delete all files   
    }
}
