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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public abstract class SettingsForm extends BaseComponent {
    
    /**
     * Shows only settings, not setting groups
     */
    private static final SettingFilter FLATTEN_SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting setting) {
            boolean group = SettingGroup.class.isAssignableFrom(setting.getClass());
            return !group;
        }
    };
    
    public abstract SettingGroup getSettings(); 
    
    public Collection getFlattenedSettings() {
        return getSettings().list(FLATTEN_SETTINGS);
    }
}
