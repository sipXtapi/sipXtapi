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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.context.MessageSource;

/**
 * Fieldset to display a collection of settings
 */
@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SettingsFieldset extends BaseComponent {
    @Parameter(required = true)
    public abstract Setting getSettings();

    public abstract void setSettings(Setting setting);

    @Parameter(defaultValue = "true")
    public abstract boolean getRenderGroupTitle();

    public abstract void setRenderGroupTitle(boolean render);

    @Parameter(defaultValue = "true")
    public abstract void setEnabled(boolean enabled);

    @Parameter(defaultValue = "true")
    public abstract void setRequiredEnabled(boolean enabled);

    public Collection<Setting> getFlattenedSettings() {
        return SettingUtil.filter(SettingFilter.ALL, getSettings());
    }

    @Persist(value = "session")
    public abstract boolean getShowAdvanced();

    public abstract void setShowAdvanced(boolean showAdvanced);

    public abstract Setting getCurrentSetting();

    public abstract void setCurrentSetting(Setting setting);

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource setting);

    public IComponent getCurrentBlock() {
        String blockName = "settingBlock";
        Setting currentSetting = getCurrentSetting();
        if (currentSetting instanceof SettingSet) {
            blockName = "groupBlock";
        } else if (currentSetting instanceof SettingArray) {
            blockName = "arrayBlock";
        }
        return getComponent(blockName);
    }

    /**
     * Collects ids of avanced settings to be used to refresh only those when toggle advanced link
     * is clicked
     * 
     * @return collection of decorated settings ids
     */
    public boolean getHasAdvancedSettings() {
        for (Setting setting : getFlattenedSettings()) {
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
    public boolean getRenderGroup() {
        Setting setting = getCurrentSetting();
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
    public boolean getRenderSetting() {
        Setting setting = getCurrentSetting();
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
    public boolean getRenderSettingPlaceholder() {
        Setting setting = getCurrentSetting();
        if (setting.isHidden()) {
            // do not render hidden seetings
            return false;
        }
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
