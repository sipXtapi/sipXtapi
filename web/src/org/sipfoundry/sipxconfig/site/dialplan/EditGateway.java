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
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * EditGateway
 */
public abstract class EditGateway extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditGateway";

    public abstract Integer getGatewayId();

    public abstract void setGatewayId(Integer id);

    public abstract Gateway getGateway();

    public abstract void setGateway(Gateway gateway);

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer id);

    public abstract void setDialPlanManager(DialPlanManager dialPlanManager);

    public abstract DialPlanManager getDialPlanManager();
    
    public abstract String getNextPage();
    
    public abstract void setNextPage(String nextPage);    

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid(cycle);
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Gateway gateway = getGateway();
        if (null != gateway) {
            return;
        }
        Integer id = getGatewayId();
        if (null != id) {
            DialPlanManager manager = getDialPlanManager();
            gateway = manager.getGateway(id);
        } else {
            gateway = new Gateway();
        }
        setGateway(gateway);
    }

    void saveValid(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        Gateway gateway = getGateway();
        Integer id = getGatewayId();
        if (id == null) {
            manager.addGateway(gateway);
        } else {
            manager.updateGateway(id, gateway);
        }
        // attach gateway to current rule
        Integer ruleId = getRuleId();
        if (null != ruleId) {
            FlexibleDialPlan flexDialPlan = manager.getFlexDialPlan();
            DialingRule rule = flexDialPlan.getRule(ruleId);
            rule.addGateway(gateway);
        }
        cycle.activate(getNextPage());
    }
}
