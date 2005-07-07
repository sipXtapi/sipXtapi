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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;

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

    public abstract void setDialPlanManager(DialPlanContext dialPlanManager);

    public abstract DialPlanContext getDialPlanManager();
    
    public abstract String getNextPage();
    
    public abstract void setNextPage(String nextPage);
    
    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        // TextArea validators have to be called explicitly
        StringSizeValidator descriptionValidator = (StringSizeValidator) getBeans().getBean("descriptionValidator");
        descriptionValidator.validate(delegate);
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
            DialPlanContext manager = getDialPlanManager();
            gateway = manager.getGateway(id);
        } else {
            gateway = new Gateway();
        }
        setGateway(gateway);
    }

    void saveValid(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanManager();
        Gateway gateway = getGateway();
        manager.storeGateway(gateway);
        // attach gateway to current rule
        Integer ruleId = getRuleId();
        if (null != ruleId) {
            DialingRule rule = manager.getRule(ruleId);
            rule.addGateway(gateway);
            manager.storeRule(rule);
        }
        cycle.activate(getNextPage());
    }
}
