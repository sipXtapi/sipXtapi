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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;

public abstract class TabNavigation extends BaseComponent {
    public abstract void setSelectedTab(String section);

    public void activateTab(String tabName) {
        setSelectedTab(tabName);
    }

    public void submitAndActivateTab(String tabName) {
        if (TapestryUtils.isValid(this)) {
            setSelectedTab(tabName);
        }
    }
}
