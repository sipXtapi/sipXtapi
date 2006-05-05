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

import org.apache.commons.lang.enums.Enum;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.components.EnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminderPanel;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends BasePage {
    public static final Enum[] OPTIONS = {
        ConfigFileType.MAPPING_RULES, ConfigFileType.FALLBACK_RULES, ConfigFileType.AUTH_RULES
    };

    public static final String PAGE = "ActivateDialPlan";

    public abstract ConfigFileType getSelected();

    public abstract DialPlanContext getDialPlanContext();

    public String getXml() {
        ConfigGenerator generator = getDialPlanContext().getGenerator();
        ConfigFileType type = getSelected();
        return generator.getFileContent(type);
    }

    @SuppressWarnings("unused")
    public void setXml(String xml_) {
        // ignore xml - read only field
    }

    public Process[] getAffectedProcesses() {
        return new Process[] {
            Process.REGISTRAR, Process.AUTH_PROXY
        };
    }

    public IPropertySelectionModel getFileSelectionModel() {
        EnumPropertySelectionModel model = new EnumPropertySelectionModel();
        model.setOptions(OPTIONS);
        return model;
    }

    public String activate() {
        DialPlanContext manager = getDialPlanContext();
        manager.activateDialPlan();
        RestartReminderPanel reminder = (RestartReminderPanel) getComponent("reminder");
        reminder.restart();
        return EditFlexibleDialPlan.PAGE;
    }
}
