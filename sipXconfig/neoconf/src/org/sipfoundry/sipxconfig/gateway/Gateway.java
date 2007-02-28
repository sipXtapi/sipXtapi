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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Gateway
 */
public class Gateway extends BeanWithSettings implements NamedObject {
    private String m_name;

    private String m_address;

    private String m_prefix;

    private String m_description;

    private String m_beanId;

    private String m_modelId;

    private String m_serialNumber;

    private String m_tftpRoot;

    private GatewayModel m_model;

    private ModelSource<GatewayModel> m_modelSource;

    private DeviceVersion m_version;

    private ProfileGenerator m_profileGenerator;

    private GatewayCallerAliasInfo m_callerAliasInfo = new GatewayCallerAliasInfo();

    public Gateway() {
    }

    public Gateway(GatewayModel model) {
        setModel(model);
    }

    @Override
    public void initialize() {
    }

    public void generateProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
    }

    public void removeProfiles() {
        // do nothing for generic gateways - we do not generate profile for it
    }

    public DeviceVersion getDeviceVersion() {
        return m_version;
    }

    public void setDeviceVersion(DeviceVersion version) {
        m_version = version;
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

    public void setProfileGenerator(ProfileGenerator profileGenerator) {
        m_profileGenerator = profileGenerator;
    }
    
    protected ProfileGenerator getProfileGenerator() {
        return m_profileGenerator;
    }

    public String getBeanId() {
        return m_beanId;
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    public void setModel(GatewayModel model) {
        m_model = model;
        m_modelId = m_model.getModelId();
    }

    public GatewayModel getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (m_modelId == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(m_modelId);
        return m_model;
    }

    public String getModelId() {
        return m_modelId;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    /**
     * Used to set header parameter route in fallback rules when generating rules for this
     * gateway.
     */
    public String getRoute() {
        return null;
    }

    public GatewayCallerAliasInfo getCallerAliasInfo() {
        return m_callerAliasInfo;
    }

    public void setCallerAliasInfo(GatewayCallerAliasInfo callerAliasInfo) {
        m_callerAliasInfo = callerAliasInfo;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }

    /**
     * Prepends gateway specific call pattern to call pattern.
     */
    public String getCallPattern(String callPattern) {
        if (StringUtils.isEmpty(m_prefix)) {
            return callPattern;
        }
        return m_prefix + callPattern;
    }

    protected Object clone() throws CloneNotSupportedException {
        Gateway clone = (Gateway) super.clone();
        clone.m_callerAliasInfo = (GatewayCallerAliasInfo) m_callerAliasInfo.clone();
        return clone;
    }

    public void setGatewayModelSource(ModelSource<GatewayModel> modelSource) {
        m_modelSource = modelSource;
    }
}
