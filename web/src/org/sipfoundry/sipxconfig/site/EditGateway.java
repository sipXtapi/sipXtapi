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
import org.apache.tapestry.valid.IValidationDelegate;

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

    public abstract void setCurrentDialPlanId(Integer dialPlanId);

    public abstract Integer getCurrentDialPlanId();

    public abstract void setAddMode(boolean addMode);

    public abstract boolean getAddMode();

    public abstract void setDialPlanManager(DialPlanManager dialPlanManager);

    public abstract DialPlanManager getDialPlanManager();

    public abstract void setEmergencyGateway(boolean emergencyGateway);

    public abstract boolean getEmergencyGateway();

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid(cycle);
        }
    }

    void saveValid(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        Gateway gateway = getGateway();
        if (getAddMode()) {
            manager.addGateway(gateway);
        } else {
            manager.updateGateway(gateway);
        }
        // also attach gateway to the plan if plan is present
        Integer planId = getCurrentDialPlanId();
        DialPlan plan = manager.getDialPlan(planId);
        if (plan != null) {
            plan.addGateway(gateway, getEmergencyGateway());
            EditDialPlan editDialPlanPage = (EditDialPlan) cycle.getPage(EditDialPlan.PAGE);
            editDialPlanPage.setDialPlan(plan);
            cycle.activate(editDialPlanPage);
        } else {
            cycle.activate(ListGateways.PAGE);
        }
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(getNextPageName());
    }

    /**
     * Determines which page we need to get back to It should be DialPlan page
     * if we are editing gateways for dial plan And ListGateways page if we just
     * editing the gateway
     * 
     * @return name of the page that should be activated
     */
    private String getNextPageName() {
        return null != getCurrentDialPlanId() ? EditDialPlan.PAGE : ListGateways.PAGE;
    }
}
