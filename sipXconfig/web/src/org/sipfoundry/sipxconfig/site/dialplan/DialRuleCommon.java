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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.gateway.GatewaysPanel;
import org.sipfoundry.sipxconfig.site.gateway.SelectGateways;

/**
 * EditDialRule
 */
public abstract class DialRuleCommon extends BaseComponent {
    public abstract DialPlanContext getDialPlanContext();

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer ruleId);

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public abstract RuleValidator getValidRule();

    public abstract boolean isRenderGateways();

    public IPage addGateway(IRequestCycle cycle) {
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Integer id = getRuleId();
        editGatewayPage.setRuleId(id);
        editGatewayPage.setGatewayId(null);
        editGatewayPage.setReturnPage(getPage());
        return editGatewayPage;
    }

    public IPage selectGateway(IRequestCycle cycle) {
        Integer id = getRuleId();
        SelectGateways selectGatewayPage = (SelectGateways) cycle.getPage(SelectGateways.PAGE);
        selectGatewayPage.setRuleId(id);
        selectGatewayPage.setNextPage(cycle.getPage().getPageName());
        return selectGatewayPage;
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        RuleValidator ruleValidator = getValidRule();
        try {
            ruleValidator.validate((IFormComponent) component.getComponent("enabled"), null,
                    getRule());
        } catch (ValidatorException e) {
            delegate.record(e);
        }
        return !delegate.getHasErrors();
    }

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    private void saveValid() {
        DialingRule rule = getRule();
        getDialPlanContext().storeRule(rule);
        Integer id = getRule().getId();
        setRuleId(id);
    }

    public void formSubmit(IRequestCycle cycle) {
        if (!isValid()) {
            return;
        }
        gatewaysPanelSubmit(cycle);
    }

    /**
     * Process submit request for the gatewaysPanel component. Would be better to make the
     * component itself process the submit request but I did not find any way to do that.
     * 
     * @param cycle current request cycle
     */
    private void gatewaysPanelSubmit(IRequestCycle cycle) {
        if (!isRenderGateways()) {
            return;
        }
        GatewaysPanel panel = (GatewaysPanel) getComponent("gatewaysPanel");
        if (panel.onFormSubmit()) {
            saveValid();
            // do not do anything else
            return;
        }
        // panel can set or select action
        // rules must be saved before such action is executed
        IActionListener action = panel.getAction();
        if (null != action) {
            saveValid();
            // HACK: clean rule if persistent - action will take us to some other page
            setRule(null);
            action.actionTriggered(panel, cycle);
        }
    }
}
