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

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;


/**
 * One UploadSpecification instance for all types of uploads
 */
public class UploadSpecification extends DeviceDescriptor {
    private String m_modelFilePath;

    public UploadSpecification() {
    }

    public UploadSpecification(String beanId) {
        super(beanId);
    }
    
    public UploadSpecification(String beanId, String specificationId) {
        super(beanId, specificationId);
    }
    
    /**
     * @return getModelId()
     */
    public String getSpecificationId() {
        return getModelId();
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
