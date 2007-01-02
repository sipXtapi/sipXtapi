/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;

/**
 * Base class for all extended setting navigation classes
 */
public abstract class NavigationWithSettings extends BaseComponent {
    @Parameter
    public abstract String getHeading();

    /**
     * Name of the currently selected tab.
     */
    @Parameter(required = true)
    public abstract String getTab();

    public abstract void setSettings(Setting settings);

    public abstract Setting getSettings();

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource messageSource);

    public String getCurrentSettingLabel() {
        Setting setting = getCurrentSetting();
        return TapestryUtils.getModelMessage(this, getMessageSource(), setting.getLabelKey(),
                setting.getLabel());
    }

    /**
     * Returns property to be used as page subtitle (heading). If heading was passed explicitely
     * we will use it, otherwise we will use root setting localized label.
     */
    public String getTabHeading() {
        String heading = getHeading();
        if (heading != null) {
            return heading;
        }
        Setting setting = getSettings().getSetting(getTab());        
        return TapestryUtils.getModelMessage(this, getMessageSource(), setting.getLabelKey(),
                setting.getLabel());
    }
}
