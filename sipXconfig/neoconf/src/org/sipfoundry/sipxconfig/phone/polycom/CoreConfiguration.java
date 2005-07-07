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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating ipmid.cfg
 */
public class CoreConfiguration extends ConfigurationFile {
    
    private static PatternSettingFilter s_callSettings = new PatternSettingFilter();
    static {
        s_callSettings.addExcludes("/call/donotdisturb.*$");
        s_callSettings.addExcludes("/call/shared.*$");
    }
    
    public CoreConfiguration(PolycomPhone phone) {
        super(phone);
    }
    
    protected void addContext(VelocityContext context) {
        super.addContext(context);
        Setting endpointSettings = getPhone().getSettings();
        Setting call = endpointSettings.getSetting(CALL_SETTINGS);
        context.put("call", FilterRunner.filter(s_callSettings, call));
    }
}

