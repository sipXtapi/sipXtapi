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
package org.sipfoundry.sipxconfig.setting;



public interface SettingFilter {

    /**
     * Effectively returns all settings and setting groups recursively
     * not including root setting.
     */
    public static final SettingFilter ALL = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting_) {
            return true;
        }
    };

    public boolean acceptSetting(Setting root, Setting setting);
}
