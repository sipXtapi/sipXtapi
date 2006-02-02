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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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
    private static final Log LOG = LogFactory.getLog(Upload.class);
    private String m_name;
    private String m_description;
    private UploadSpecification m_specification;
    private ModelFilesContext m_modelFilesContext;
    private String m_beanId;
    private String m_uploadRootDirectory;
    private String m_destinationDirectory;
    private boolean m_deployed;
    private String m_directoryId;

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
    
    void setDirectoryId(String directory) {
        m_directoryId = directory;
    }
    
    String getDirectoryId() {
        return m_directoryId != null ? m_directoryId : getId().toString();
    }

    public boolean isDeployed() {
        return m_deployed;
    }

    /**
     * @deprecated Not deprecated, but should only be called by DB marshalling. See deploy and undeploy 
     */
    public void setDeployed(boolean enabled) {
        m_deployed = enabled;
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
                fileType.setDirectory(getUploadDirectory());
            }
        }
    }
    
    private class FileDeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    deployFile(filename);
                }
            }
        }
    }

    private class FileUndeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    File f = new File(getDestinationDirectory(), filename);
                    f.delete();
                }
            }
        }
    }
    
    private void deployFile(String file) {
        InputStream from;
        try {
            from = new FileInputStream(new File(getUploadDirectory(), file));
            File destDir = new File(getDestinationDirectory());
            destDir.mkdirs();
            OutputStream to = new FileOutputStream(new File(destDir, file));
            IOUtils.copy(from, to);
        } catch (IOException e) {
            throw new RuntimeException(e);
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
    
    public void setDestinationDirectory(String destinationDirectory) {
        m_destinationDirectory = destinationDirectory;
    }
    
    public String getDestinationDirectory() {
        return m_destinationDirectory;
    }

    /**
     * delete all files
     */       
    public void remove() {
        undeploy();
        File uploadDirectory = new File(getUploadDirectory());
        try {
            FileUtils.deleteDirectory(uploadDirectory);
        } catch (IOException cantDelete) {
            LOG.error("Could not remove uploaded files", cantDelete);
        }
    }
    
    public void setUploadRootDirectory(String uploadDirectory) {
        m_uploadRootDirectory = uploadDirectory;
    }
    
    public String getUploadDirectory() {
        return m_uploadRootDirectory + '/' + getDirectoryId();
    }

    public void deploy() {
        getSettings().acceptVisitor(new FileDeployer());        
        m_deployed = true;
    }
    
    public void undeploy() {
        getSettings().acceptVisitor(new FileUndeployer());        
        m_deployed = false;
    }
}
