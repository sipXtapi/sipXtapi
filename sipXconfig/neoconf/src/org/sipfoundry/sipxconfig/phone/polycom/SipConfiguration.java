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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Responsible for generating ipmid.cfg
 */
public class SipConfiguration extends ProfileContext {
    private static PatternSettingFilter s_callSettings = new PatternSettingFilter();
    static {
        s_callSettings.addExcludes("call/donotdisturb.*$");
        s_callSettings.addExcludes("call/shared.*$");
    }

    public SipConfiguration(BeanWithSettings device) {
        super(device);
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        Setting endpointSettings = getDevice().getSettings();
        Setting call = endpointSettings.getSetting(PolycomPhone.CALL);
        Collection items = SettingUtil.filter(s_callSettings, call);
        context.put(PolycomPhone.CALL, items);
        return context;
    }
}
