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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * EditGateway
 */
public abstract class EditGateway extends BasePage {
    public static final String PAGE = "EditGateway";

    public abstract void setGateway(Gateway gateway);

    public abstract Gateway getGateway();

    public abstract void setCurrentDialPlan(DialPlan dialPlan);

    public abstract DialPlan getCurrentDialPlan();

    public abstract void setAddMode(boolean addMode);

    public abstract boolean getAddMode();

    public abstract void setDialPlanManager(DialPlanManager dialPlanManager);

    public abstract DialPlanManager getDialPlanManager();

    public abstract void setEmergencyGateway(boolean emergencyGateway);

    public abstract boolean getEmergencyGateway();

    public void save(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        Gateway gateway = getGateway();
        if (getAddMode()) {
            manager.addGateway(gateway);
        } else {
            manager.updateGateway(gateway);
        }
        // also attach gateway to the plan if plan is present
        DialPlan plan = getCurrentDialPlan();
        if (plan != null) {
            plan.addGateway(gateway, getEmergencyGateway());
        }
        cycle.activate(getNextPageName());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(getNextPageName());
    }

    /**
     * Determines which page we need to get back to It should be DialPlan page
     * if we are editing gateways for dial plan And ListGateways page if we just
     * editing the gateway
     * 
     * @return
     */
    private String getNextPageName() {
        return null != getCurrentDialPlan() ? EditDialPlan.PAGE : ListGateways.PAGE;
    }
}
