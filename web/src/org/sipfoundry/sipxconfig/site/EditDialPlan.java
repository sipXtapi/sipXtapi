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

    public abstract Integer getDialPlanId();

    public abstract void setDialPlanId(Integer id);

    public abstract DialPlan getDialPlan();

    public abstract void setDialPlan(DialPlan plan);

    public void pageBeginRender(PageEvent event_) {
        DialPlan plan = getDialPlan();
        if (null != plan) {
            return;
        }
        Integer id = getDialPlanId();
        if (null != id) {
            DialPlanManager manager = getDialPlanManager();
            plan = manager.getDialPlan(id);
        } else {
            plan = new DialPlan();
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
        Integer id = getDialPlanId();
        id.toString();
        editGatewayPage.setCurrentDialPlanId(id);
        editGatewayPage.setEmergencyGateway(emergency);
        editGatewayPage.setGatewayId(null);
        cycle.activate(editGatewayPage);
    }

    void saveValid(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        DialPlan dialPlan = getDialPlan();
        Integer id = getDialPlanId();
        if (null == id) {
            manager.addDialPlan(dialPlan);
        } else {
            manager.updateDialPlan(id, dialPlan);
        }
        cycle.activate(ListDialPlans.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListDialPlans.PAGE);
    }
}
