/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;



public abstract class DeviceDescriptor {
    private String m_modelFilePath;

    private String m_beanId;

    private String m_label;

    private String m_modelId;

    private DeviceVersion[] m_versions = new DeviceVersion[0];
    
    public DeviceDescriptor() {        
    }
    
    public DeviceDescriptor(String beanId) {
        setBeanId(beanId);
    }
    
    public DeviceDescriptor(String beanId, String modelId) {
        this(beanId);
        setModelId(modelId);
    }
    

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    /**
     * Spring bean name, NOTE: You cannot reuse java class to multiple spring beans. One
     * class/bean but a spring bean can handle multiple models
     */
    public String getBeanId() {
        return m_beanId;
    }

    /**
     * User identifiable label for this model
     */
    public String getLabel() {
        return m_label;
    }

    public String getModelId() {
        return m_modelId;
    }

    /**
     * @return true if phones with this model have a serial number, false otherwise
     */
    public boolean getHasSerialNumber() {
        return true;
    }
    
    protected void setVersions(DeviceVersion[] versions) {
        m_versions = versions;
    }
    
    public DeviceVersion[] getVersions() {
        return m_versions;
    }    

    /**
     * @return File name with upload settings describing files to upload.  Relative to /etc/sipxpbx
     */
    public String getModelFilePath() {
        return m_modelFilePath;
    }

    public void setModelFilePath(String modelFilePath) {
        m_modelFilePath = modelFilePath;
    }
}
