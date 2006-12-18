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

import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.site.common.NavigationWithSettings;

public abstract class SettingsNavigation extends NavigationWithSettings {
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
}
