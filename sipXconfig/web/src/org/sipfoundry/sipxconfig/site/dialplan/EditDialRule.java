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
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;
import org.sipfoundry.sipxconfig.common.Permission;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.BeanFactoryGlobals;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.gateway.GatewaysPanel;
import org.sipfoundry.sipxconfig.site.gateway.SelectGateways;

/**
 * EditDialRule
 */
public abstract class EditDialRule extends BasePage implements PageRenderListener {
    /**
     * list of permission types allowed for long distance permission, used in permssions modle on
     * a long distance page
     */
    public static final Permission[] LONG_DISTANCE_PERMISSIONS = {
        Permission.LONG_DISTANCE_DIALING, Permission.RESTRICTED_DIALING,
        Permission.TOLL_FREE_DIALING
    };

    private DialingRuleType m_ruleType;

    public abstract DialPlanContext getDialPlanContext();

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer ruleId);

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public Permission[] getCallHandlingPermissions() {
        return Permission.CALL_HANDLING.getChildren();
    }

    public DialingRuleType getRuleType() {
        return m_ruleType;
    }

    public void setRuleType(DialingRuleType dialingType) {
        m_ruleType = dialingType;
    }

    public void pageBeginRender(PageEvent event_) {
        DialingRule rule = getRule();
        if (null != rule) {
            return;
        }
        Integer id = getRuleId();
        if (null != id) {
            DialPlanContext manager = getDialPlanContext();
            rule = manager.getRule(id);
        } else {
            rule = createNewRule();
        }
        setRule(rule);

        // Ignore the callback passed to us for now because we're navigating
        // to unexpected places. Always go to the EditFlexibleDialPlan plan.
        setCallback(new PageCallback(EditFlexibleDialPlan.PAGE));
    }

    protected DialingRule createNewRule() {
        DialingRuleFactory ruleFactory = getDialPlanContext().getRuleFactory();
        DialingRuleType ruleType = getRuleType();
        return ruleFactory.create(ruleType);
    }

    public void addGateway(IRequestCycle cycle) {
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Integer id = getRuleId();
        editGatewayPage.setRuleId(id);
        editGatewayPage.setGatewayId(null);
        editGatewayPage.setCallback(new PageCallback(this));
        cycle.activate(editGatewayPage);
    }

    public void selectGateway(IRequestCycle cycle) {
        Integer id = getRuleId();
        SelectGateways selectGatewayPage = (SelectGateways) cycle.getPage(SelectGateways.PAGE);
        selectGatewayPage.setRuleId(id);
        selectGatewayPage.setNextPage(cycle.getPage().getPageName());
        cycle.activate(selectGatewayPage);
    }

    /**
     * Go to emergency routing page. Set callback to return to this page. Used only in
     * EditEmergencyRouting rule
     */
    public void emergencyRouting(IRequestCycle cycle) {
        PageCallback callback = new PageCallback(getPage());
        EditEmergencyRouting page = (EditEmergencyRouting) cycle
                .getPage(EditEmergencyRouting.PAGE);
        page.setCallback(callback);
        cycle.activate(page);
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);
        // TODO: check if it can be changed to page bean
        BeanFactoryGlobals globals = (BeanFactoryGlobals) getGlobal();
        RuleValidator ruleValidator = (RuleValidator) globals.get("validRule");
        ruleValidator.validate(getRule(), delegate, (IFormComponent) component
                .getComponent("enabled"));
        return !delegate.getHasErrors();
    }

    public void commit(IRequestCycle cycle_) {
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
     * Process submit request for the gatewaysPanel component.  Would be better to make the
     * component itself process the submit request but I did not find any way to do that.
     * 
     * @param cycle current request cycle
     */
    private void gatewaysPanelSubmit(IRequestCycle cycle) {
        // NOTE: do not use getComponent("gatewaysPanel") here - it throws exception if component
        // is not present
        GatewaysPanel panel = (GatewaysPanel) getComponents().get("gatewaysPanel");
        if (null == panel) {
            // no component - nothing to do
            return;
        }
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
