/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueHandler;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;

public class AudioCodesGatewayDefaults implements SettingValueHandler {
    private AudioCodesGateway m_gateway;
    private DeviceDefaults m_defaults;

    AudioCodesGatewayDefaults(AudioCodesGateway gateway, DeviceDefaults defaults) {
        m_gateway = gateway;
        m_defaults = defaults;
    }

    @SettingEntry(path = "SIP/SIPGATEWAYNAME")
    public String getGatewayName() {
        return m_gateway.getDefaults().getDomainName();
    }

    @SettingEntry(path = "SIP/SIPDESTINATIONPORT")
    public String getDestinationPort() {
        return m_gateway.getDefaults().getProxyServerSipPort();
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue value = null;
        String path = setting.getPath();
        AudioCodesModel model = (AudioCodesModel) m_gateway.getModel();
        if (path.equals(model.getProxyNameSetting())) {
            value = new SettingValueImpl(m_defaults.getDomainName());
        } else if (path.equals(model.getProxyIpSetting())) {
            value = new SettingValueImpl(m_defaults.getProxyServerAddr());
        }

        return value;
    }
}
