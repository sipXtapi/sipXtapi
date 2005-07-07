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
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends BasePage {
    public static final String PAGE = "ActivateDialPlan";

    public abstract ConfigFileType getSelected();

    public abstract DialPlanContext getDialPlanManager();

    public String getXml() {
        ConfigGenerator generator = getDialPlanManager().getGenerator();
        ConfigFileType type = getSelected();
        return generator.getFileContent(type);
    }

    public void setXml(String xml_) {
        // ignore xml - read only field
    }

    public void activate(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanManager();
        manager.activateDialPlan();
        RestartReminder restartPage = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        restartPage.setNextPage(EditFlexibleDialPlan.PAGE);
        restartPage.setProcesses(new Process[] {
            Process.REGISTRAR, Process.AUTH_PROXY
        });
        cycle.activate(restartPage);
    }
}
