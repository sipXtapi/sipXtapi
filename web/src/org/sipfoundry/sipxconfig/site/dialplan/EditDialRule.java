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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.components.GatewayTable;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;

/**
 * EditCustomeDialRule
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

    private DialingRule.Type m_ruleType;

    public abstract DialPlanContext getDialPlanManager();

    public abstract void setRuleId(Integer ruleId);

    public abstract Integer getRuleId();

    public abstract String getRemoveGateways();

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

    public DialingRule.Type getRuleType() {
        return m_ruleType;
    }

    public void setRuleType(DialingRule.Type dialingType) {
        m_ruleType = dialingType;
    }

    public void pageBeginRender(PageEvent event_) {
        DialingRule rule = getRule();
        if (null != rule) {
            return;
        }
        Integer id = getRuleId();
        if (null != id) {
            DialPlanContext manager = getDialPlanManager();
            rule = manager.getFlexDialPlan().getRule(id);
        } else {
            rule = createNewRule();
        }
        setRule(rule);
    }

    protected DialingRule createNewRule() {
        DialingRuleFactory ruleFactory = getDialPlanManager().getRuleFactory();
        DialingRule.Type ruleType = getRuleType();
        return ruleFactory.create(ruleType);
    }

    public void addGateway(IRequestCycle cycle) {
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Integer id = getRuleId();
        editGatewayPage.setRuleId(id);
        editGatewayPage.setGatewayId(null);
        editGatewayPage.setNextPage(cycle.getPage().getPageName());
        cycle.activate(editGatewayPage);
    }

    public void selectGateway(IRequestCycle cycle) {
        SelectGateways selectGatewayPage = (SelectGateways) cycle.getPage(SelectGateways.PAGE);
        Integer id = getRuleId();
        selectGatewayPage.setRuleId(id);
        selectGatewayPage.setNextPage(cycle.getPage().getPageName());
        cycle.activate(selectGatewayPage);
    }

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid(cycle);
            cycle.activate(EditFlexibleDialPlan.PAGE);
        }
    }

    private void saveValid(IRequestCycle cycle_) {
        FlexibleDialPlanContext plan = getDialPlanManager().getFlexDialPlan();
        DialingRule rule = getRule();
        plan.storeRule(rule);
    }

    /**
     * Handles removing gateways on submit.
     * 
     * Emergency property is set by Remove submit buttons. If it's not set at all none of this
     * buttong has been pressed and we have nothing to do.
     */
    private void removeGateways() {
        String removeGateways = getRemoveGateways();
        if (null == removeGateways) {
            return;
        }
        GatewayTable table = (GatewayTable) getComponent("gatewayTable");
        Collection selectedGateways = table.getSelections().getAllSelected();
        // remove normal gateways
        getRule().removeGateways(selectedGateways);
    }

    public void formSubmit(IRequestCycle cycle_) {
        removeGateways();
    }
}
