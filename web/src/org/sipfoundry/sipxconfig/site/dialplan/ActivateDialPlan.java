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
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends BasePage {
    public static final String PAGE = "ActivateDialPlan";

    public abstract ConfigFileType getSelected();

    public abstract DialPlanManager getDialPlanManager();

    public String getXml() {
        ConfigGenerator generator = getDialPlanManager().getGenerator();
        ConfigFileType type = getSelected();
        return generator.getFileContent(type);
    }

    public void setXml(String xml_) {
        // ignore xml - read only field
    }

    public void activate(IRequestCycle cycle_) {
        DialPlanManager manager = getDialPlanManager();
        manager.activateDialPlan();
    }
}
