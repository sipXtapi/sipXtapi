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
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.context.MessageSource;

public abstract class SettingsFieldset extends BaseComponent {
    public abstract Setting getCurrentSetting();

    public abstract Setting getSettings();

    public abstract void setSettings(Setting setting);

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource setting);

    public abstract boolean getRenderGroupTitle();

    public abstract void setRenderGroupTitle(boolean render);

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
     * Render group if it's not advanced (hidden) or if show advanced is set and group title
     * rendering is allowed
     * 
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean renderGroup(Setting setting) {
        if (!getRenderGroupTitle()) {
            // group title rendering not allowed
            return false;
        }

        return showSetting(setting);
    }

    /**
     * Render setting if it's not advanced (hidden) or if show advanced is set
     * 
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean renderSetting(Setting setting) {
        return showSetting(setting);
    }

    boolean showSetting(Setting setting) {
        if (setting.isHidden()) {
            // do not render hidden seetings
            return false;
        }
        boolean isAdvanced = SettingUtil.isAdvancedIncludingParents(getSettings(), setting);
        return !isAdvanced || getShowAdvanced();
    }

    /**
     * Returns true for advanced settings that are not rendered. Instead we render a placeholder
     * that keeps setting value in case it was modified.
     * 
     * @param setting
     * @return true if setting is not rendered (is advanced and advanced settings are not shown)
     */
    public boolean renderSettingPlaceholder(Setting setting) {
        boolean isAdvanced = SettingUtil.isAdvancedIncludingParents(getSettings(), setting);
        return isAdvanced && !getShowAdvanced();
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        // set message source only once and save it into property so that we do not have to
        // compute it every time
        if (getMessageSource() == null) {
            setMessageSource(getSettings().getMessageSource());
        }
    }

    public String getDescription() {
        Setting setting = getCurrentSetting();
        return TapestryUtils.getSettingDescription(this, setting);
    }

    public String getLabel() {
        Setting setting = getCurrentSetting();
        return TapestryUtils.getSettingLabel(this, setting);
    }
}
