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
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.components.GatewayTable;
import org.sipfoundry.sipxconfig.site.EditGateway;
import org.sipfoundry.sipxconfig.site.SelectGateways;

/**
 * EditCustomeDialRule
 */
public abstract class EditCustomDialRule extends BasePage {
    public static final String PAGE = "EditCustomDialRule";

    public abstract DialPlanManager getDialPlanManager();

    public abstract void setRuleId(Integer ruleId);

    public abstract Integer getRuleId();

    public abstract String getRemoveGateways();

    public DialingRule getCustomRule() {
        DialPlanManager manager = getDialPlanManager();
        FlexibleDialPlan flexDialPlan = manager.getFlexDialPlan();
        Integer ruleId = getRuleId();
        if (null != ruleId) {
            return flexDialPlan.getRule(ruleId);
        }
        CustomDialingRule rule = new CustomDialingRule();
        flexDialPlan.addRule(rule);
        setRuleId(rule.getId());
        return rule;
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

    public void save(IRequestCycle cycle_) {
        // TODO: implement
    }

    public void cancel(IRequestCycle cycle_) {
        // TODO: implement
    }
}
