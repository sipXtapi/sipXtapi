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

import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

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

    private VelocityEngine m_velocityEngine;
    
    @Override
    public void initialize() {        
    }

    public void generateProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
    }

    public void removeProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
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
    
    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
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

    @Override
    protected Setting loadSettings() {
        return null;
    }
}
