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
import java.util.Map;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Gateway
 */
public class Gateway extends BeanWithId {
    private String m_name;

    private String m_address;

    private String m_description;

    private Setting m_settings;

    private String m_factoryId;

    private String m_serialNumber;

    private GatewayContext m_gatewayContext;

    private ValueStorage m_valueStorage;
    
    private String m_tftpRoot;

    public void generateProfiles(Writer writer_) throws IOException {
        // generic gateways does not support generating profiles
        throw new UnsupportedOperationException();
    }

    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel() {
        return null;
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        if (m_settings == null) {
            Setting settings = getSettingModel();
            if (settings == null) {
                // it's OK for a gateway not to provide settings
                return null;
            }
            if (m_valueStorage == null) {
                m_valueStorage = new ValueStorage();
            }

            m_settings = m_valueStorage.decorate(settings);
            // TODO do we need setDefaults(); here
        }

        return m_settings;
    }

    public String getModelName() {
        Map factoryIds = m_gatewayContext.getFactoryIds();
        return (String) factoryIds.get(m_factoryId);
    }

    public void generateProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
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

    public String getFactoryId() {
        return m_factoryId;
    }

    public void setFactoryId(String factoryId) {
        m_factoryId = factoryId;
    }

    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public GatewayContext getGatewayContext() {
        return m_gatewayContext;
    }
    
    public String getTftpRoot() {
        return m_tftpRoot;
    }
    
    public void setTftpRoot(String tftpRoot) {
        m_tftpRoot = tftpRoot;
    }
}
