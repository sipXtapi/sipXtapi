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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;

import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.components.GatewayTable;

/**
 * EditCustomeDialRule
 */
public abstract class EditCustomDialRule extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditCustomDialRule";

    public abstract DialPlanManager getDialPlanManager();

    public abstract void setRuleId(Integer ruleId);

    public abstract Integer getRuleId();

    public abstract String getRemoveGateways();

    public abstract CustomDialingRule getCustomRule();

    public abstract void setCustomRule(CustomDialingRule rule);

    public void pageBeginRender(PageEvent event_) {
        CustomDialingRule rule = getCustomRule();
        if (null != rule) {
            return;
        }
        Integer id = getRuleId();
        if (null != id) {
            DialPlanManager manager = getDialPlanManager();
            rule = (CustomDialingRule) manager.getFlexDialPlan().getRule(id);
        } else {
            rule = new CustomDialingRule();
        }
        setCustomRule(rule);
    }

    public void addGateway(IRequestCycle cycle) {
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Integer id = getRuleId();
        editGatewayPage.setRuleId(id);
        editGatewayPage.setGatewayId(null);
        cycle.activate(editGatewayPage);
    }

    public void selectGateway(IRequestCycle cycle) {
        SelectGateways selectGatewayPage = (SelectGateways) cycle.getPage(SelectGateways.PAGE);
        Integer id = getRuleId();
        selectGatewayPage.setRuleId(id);
        cycle.activate(selectGatewayPage);
    }

    /**
     * Handles removing gateways on submit.
     * 
     * Emergency property is set by Remove submit buttons. If it's not set at
     * all none of this buttong has been pressed and we have nothing to do.
     */
    private void removeGateways() {
        String removeGateways = getRemoveGateways();
        if (null == removeGateways) {
            return;
        }
        GatewayTable table = (GatewayTable) getComponent("gatewayTable");
        Collection selectedGateways = table.getSelections().getAllSelected();
        // remove normal gateways
        getCustomRule().removeGateways(selectedGateways);
    }

    public void formSubmit(IRequestCycle cycle_) {
        removeGateways();
    }

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid(cycle);
            cycle.activate(EditFlexibleDialPlan.PAGE);
        }
    }

    private void saveValid(IRequestCycle cycle_) {
        FlexibleDialPlan plan = getDialPlanManager().getFlexDialPlan();
        CustomDialingRule rule = getCustomRule();
        Integer ruleId = getRuleId();
        if (null == ruleId) {
            plan.addRule(rule);
        } else {
            plan.updateRule(rule);
        }
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(EditFlexibleDialPlan.PAGE);
    }

    /**
     * Creates the pages instance for current request cycle
     * 
     * @param cycle
     * @return page
     */
    public static EditCustomDialRule getPage(IRequestCycle cycle) {
        EditCustomDialRule page = (EditCustomDialRule) cycle.getPage(PAGE);
        return page;
    }
}
