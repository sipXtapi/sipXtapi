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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditDialPlan extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditDialPlan";

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract DialPlan getDialPlan();

    public abstract void setDialPlan(DialPlan plan);

    public abstract Integer getDialPlanId();

    public abstract void setDialPlanId(Integer plan);

    public abstract void setAddMode(boolean b);

    public abstract boolean getAddMode();

    public void pageBeginRender(PageEvent event_) {
        DialPlanManager manager = getDialPlanManager();
        Integer planId = getDialPlanId();
        DialPlan plan = manager.getDialPlan(planId);
        if (null == plan) {
            plan = new DialPlan();
            if (null == planId) {
                setDialPlanId(plan.getId());
            } else {
                plan.setId(planId);
            }
        }
        setDialPlan(plan);
    }

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid(cycle);
        }
    }

    public void addGateway(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        boolean emergency = "emergency".equals(params[0]);
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        editGatewayPage.setCurrentDialPlanId(getDialPlanId());
        editGatewayPage.setEmergencyGateway(emergency);
        editGatewayPage.setAddMode(true);
        cycle.activate(editGatewayPage);
    }

    void saveValid(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        DialPlan dialPlan = getDialPlan();
        if (getAddMode()) {
            manager.addDialPlan(dialPlan);
        } else {
            manager.updateDialPlan(dialPlan);
        }
        cycle.activate(ListDialPlans.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListDialPlans.PAGE);
    }
}
