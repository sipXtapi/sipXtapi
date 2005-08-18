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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public abstract class SettingsFieldset extends BaseComponent {
    public abstract Setting getCurrentSetting();

    public abstract Setting getSettings();

    public abstract void setSettings(Setting setting);

    public Collection getFlattenedSettings() {
        return SettingUtil.filter(SettingFilter.ALL, getSettings());
    }

    public abstract boolean getShowAdvanced();

    public abstract void setShowAdvanced(boolean showAdvanced);

    /**
     * Collects ids of avanced settings to be used to refresh only those when toggle advanced link
     * is clicked
     * 
     * @return collection of decorated settings ids
     */
    public boolean getHasAdvancedSettings() {
        Collection flattenedSettings = getFlattenedSettings();
        for (Iterator i = flattenedSettings.iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            if (setting.isAdvanced()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Render setting if it's not advanced (hidden) or if show advanced is set
     * 
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean renderSetting(Setting setting) {
        if (setting.isHidden()) {
            // do not render hidden seetings
            return false;
        }
        return !setting.isAdvanced() || getShowAdvanced();
    }
}
