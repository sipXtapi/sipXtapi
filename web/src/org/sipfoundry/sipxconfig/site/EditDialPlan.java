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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.components.GatewayTable;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditDialPlan extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditDialPlan";

    public static final String EMERGENCY = "emergency";

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract Integer getDialPlanId();

    public abstract void setDialPlanId(Integer id);

    public abstract DialPlan getDialPlan();

    public abstract void setDialPlan(DialPlan plan);

    public abstract String getRemoveGateways();

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
        boolean emergency = EMERGENCY.equals(params[0]);
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

    public void formSubmit(IRequestCycle cycle_) {
        if (!isValid()) {
            return;
        }
        removeGateways();
    }

    /**
     * Handles removing gateways on submit.
     * 
     * Emergency property is set by Remove submit buttons. If it's not set at
     * all none of this buttong has been pressed and we have nothing to do.
     */
    private void removeGateways() {
        // TODO: handle submit in other way
        String removeGateways = getRemoveGateways();
        if (null == removeGateways) {
            return;
        }
        boolean emergency = EMERGENCY.equals(removeGateways);
        GatewayTable table = getGatewayTable(emergency);
        Collection selectedGateways = table.getSelections().getAllSelected();
        // remove normal gateways
        DialPlan plan = getDialPlan();
        plan.removeGateways(selectedGateways, emergency);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListDialPlans.PAGE);
    }

    /**
     * @return appropriate gateway table base on an emergency parameter
     */
    private GatewayTable getGatewayTable(boolean emergency) {
        String gatewayTableId = emergency ? "emergencyGatewayTable" : "gatewayTable";
        return (GatewayTable) getComponent(gatewayTableId);
    }
}
