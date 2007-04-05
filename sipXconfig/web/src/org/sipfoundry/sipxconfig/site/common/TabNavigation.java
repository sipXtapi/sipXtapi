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
package org.sipfoundry.sipxconfig.site.common;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class TabNavigation extends BaseComponent {
    
    public abstract String getCurrentTab();
    
    @Parameter(required = true)
    public abstract Collection getTabNames();
    
    @Parameter()
    public abstract void setSelectedTab(String section);
    
    @Parameter(required = false, defaultValue = "false")
    public abstract boolean isSubmit();

    public void activateTab(String tabName) {
        setSelectedTab(tabName);
    }

    public void submitAndActivateTab(String tabName) {
        if (TapestryUtils.isValid(this)) {
            setSelectedTab(tabName);
        }
    }
    
    public String getCurrentTabLabel() {
        return getContainer().getMessages().getMessage("tab." + getCurrentTab());
    }
}
