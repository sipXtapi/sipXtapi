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
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

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

    private DialingRuleType m_ruleType;

    public abstract DialPlanContext getDialPlanManager();

    public abstract void setRuleId(Integer ruleId);

    public abstract Integer getRuleId();

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

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
            DialPlanContext manager = getDialPlanManager();
            rule = manager.getRule(id);
        } else {
            rule = createNewRule();
        }
        setRule(rule);
    }

    protected DialingRule createNewRule() {
        DialingRuleFactory ruleFactory = getDialPlanManager().getRuleFactory();
        DialingRuleType ruleType = getRuleType();
        return ruleFactory.create(ruleType);
    }

    public void addGateway(IRequestCycle cycle) {
        if (!isValid()) {
            return;
        }
        saveValid();
        EditGateway editGatewayPage = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Integer id = getRuleId();
        editGatewayPage.setRuleId(id);
        editGatewayPage.setGatewayId(null);
        editGatewayPage.setNextPage(cycle.getPage().getPageName());
        cycle.activate(editGatewayPage);
    }

    public void selectGateway(IRequestCycle cycle) {
        if (!isValid()) {
            return;
        }
        saveValid();
        Integer id = getRuleId();
        SelectGateways selectGatewayPage = (SelectGateways) cycle.getPage(SelectGateways.PAGE);
        selectGatewayPage.setRuleId(id);
        selectGatewayPage.setNextPage(cycle.getPage().getPageName());
        cycle.activate(selectGatewayPage);
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid();
            cycle.activate(EditFlexibleDialPlan.PAGE);
        }
    }

    private void saveValid() {
        DialingRule rule = getRule();
        getDialPlanManager().storeRule(rule);
        Integer id = getRule().getId();
        setRuleId(id);
    }

    public void formSubmit(IRequestCycle cycle_) {
        if (!isValid()) {
            return;
        }
        // NOTE: do not use getComponent("gatewayPanel") here - it throws exception if component
        // is not present
        GatewaysPanel panel = (GatewaysPanel) getComponents().get("gatewaysPanel");
        if (null != panel && panel.onFormSubmit()) {
            saveValid();
        }
    }
}
