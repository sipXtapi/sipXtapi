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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;

public abstract class SettingsNavigation extends BaseComponent {
    public abstract Setting getSettings();

    public abstract Setting getCurrentSetting();

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource messageSource);

    public abstract void setTab(String section);

    public void activateTab(String section) {
        setTab(section);
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        // set message source only once and save it into property so that we do not have to
        // compute it every time
        if (getMessageSource() == null) {
            setMessageSource(getSettings().getMessageSource());
        }
    }

    public String getLabel() {
        Setting setting = getCurrentSetting();
        MessageSource modelMessages = getMessageSource();
        String defaultMessage = setting.getLabel();
        if (modelMessages != null) {
            return modelMessages.getMessage(setting.getLabelKey(), null, defaultMessage,
                    getPage().getLocale());
        }
        return defaultMessage;
    }
}
