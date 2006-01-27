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

import java.io.File;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * Describing the files required to track and manage a vendor's firmware files
 */
public class Upload extends BeanWithSettings {
    private String m_name;
    private String m_description;
    private UploadSpecification m_specification;
    private ModelFilesContext m_modelFilesContext;
    private String m_beanId;
    private UploadDestination m_destination;

    public Upload() {
        this(UploadSpecification.UNMANAGED);
    }

    protected Upload(String beanId) {
        m_beanId = beanId;
    }

    protected Upload(UploadSpecification specification) {
        m_beanId = specification.getBeanId();
        m_specification = specification;
    }

    public UploadSpecification getSpecification() {
        return m_specification;
    }

    /**
     * @return ids used in PhoneFactory
     */
    public String getBeanId() {
        return m_beanId;
    }

    /**
     * Internal, do not call this method. Hibnerate property declared update=false, but still
     * required method be defined.
     */
    public void setBeanId(String illegal_) {
    }

    public String getSpecificationId() {
        return m_specification.getSpecificationId();
    }
    
    public void setSpecificationId(String specificationId) {
        m_specification = UploadSpecification.getSpecificationById(getBeanId() 
                + StringUtils.defaultString(specificationId));
    }

    public Setting getSettingModel() {
        Setting model = super.getSettingModel();
        if (model == null) {
            Setting master = m_modelFilesContext.loadModelFile("upload.xml", m_specification
                    .getSpecificationId());
            if (master != null) {    
                model = master.copy();
                setSettingModel(model);
            }
        }
        return model;
    }

    protected void defaultSettings() {
        getSettings().acceptVisitor(new UploadDirectorySetter());
    }

    private class UploadDirectorySetter extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                String uploadDir = getDestination().getUploadDirectory();
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

    public ModelFilesContext getModelFilesContext() {
        return m_modelFilesContext;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }
    
    public void setDestination(UploadDestination destination) {
        m_destination = destination;
    }
    
    public UploadDestination getDestination() {
        return m_destination;
    }

    /**
     * delete all files
     */       
    public void remove() {
        getSettings().acceptVisitor(new DeleteUpload());
    }
    
    private class DeleteUpload extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting && setting.getValue() != null) {
                String uploadDir = getDestination().getUploadDirectory();
                File file = new File(uploadDir + File.separator + setting.getValue());
                file.delete();
            }
        }
    }    
}
