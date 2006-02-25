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
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SettingsNavigation extends BaseComponent {
    public abstract void setTab(String section);

    public void activateTab(IRequestCycle cycle) {
        Object[] params = cycle.getListenerParameters();

        String section = (String) TapestryUtils.assertParameter(String.class, params, 0);
        setTab(section);
    }
}
