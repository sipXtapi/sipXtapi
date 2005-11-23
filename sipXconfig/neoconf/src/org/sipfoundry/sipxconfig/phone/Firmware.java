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
    private FirmwareManufacturer m_manufacturer;
    private ModelFilesContext m_modelFilesContext;
    private Integer m_uniqueUploadId;
    private String m_uploadParentDirectory;

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
                fileType.setDirectory(getUploadDirectory());
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
    
    /**
     * Where uploads exists for this firmware instance
     */
    public String getUploadDirectory() {
        return getUploadParentDirectory() + '/' + getUniqueUploadId();
    }
    
    /**
     * Parent uploads exists for this firmware instance
     */
    public String getUploadParentDirectory() {
        return m_uploadParentDirectory;
    }

    public void setUploadParentDirectory(String uploadParentDirectory) {
        m_uploadParentDirectory = uploadParentDirectory;
    }

    public Integer getUniqueUploadId() {
        return isNew() ? m_uniqueUploadId : getId();
    }

    public void setUniqueUploadId(Integer uniqueUploadId) {
        m_uniqueUploadId = uniqueUploadId;
    }
    
    public void setIdToUniqueUploadId() {
        super.setIdWithProtectedAccess(getUniqueUploadId());
    }
    
    public void deliver() {
    }    

    public void remove() {
    }    
    
    /* Work in progress
    public void deliver() {
        FileDelivery delivery = getManufacturer().getFileDelivery();
        File[] files = getFiles();
        File directory = new File(getUploadDirectory());
        for (int i = 0; i < files.length; i++) {
            delivery.deliverFile(directory, files[i]);
        }
    }
    
    File[] getFiles() {
        FileLister lister = new FileLister();
        getSettings().acceptVisitor(lister);
        File[] files = (File[]) lister.m_files.toArray(new File[0]);
        return files;
    }
    
    public void remove() {
        FileDelivery delivery = getManufacturer().getFileDelivery();
        File[] files = getFiles();
        File directory = new File(getUploadDirectory());
        for (int i = 0; i < files.length; i++) {
            delivery.removeFile(directory, files[i]);
        }
    }

    private class FileLister extends AbstractSettingVisitor {
        private List m_files = new ArrayList();
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting && setting.getValue() != null) {
                m_files.add(new File(setting.getValue()));
            }
        }
    }
    */
}
