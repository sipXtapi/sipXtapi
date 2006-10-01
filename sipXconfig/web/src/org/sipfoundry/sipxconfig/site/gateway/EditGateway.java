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
package org.sipfoundry.sipxconfig.site.gateway;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.SettingSet;

/**
 * EditGateway
 */
public abstract class EditGateway extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "EditGateway";

    public abstract Integer getGatewayId();

    public abstract void setGatewayId(Integer id);

    public abstract Gateway getGateway();

    public abstract void setGateway(Gateway gateway);

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer id);

    public abstract void setGatewayModel(PhoneModel model);

    public abstract PhoneModel getGatewayModel();

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract String getCurrentSettingSetName();

    public abstract void setCurrentSettingSet(SettingSet currentSettingSet);

    public abstract SettingSet getCurrentSettingSet();

    public void pageBeginRender(PageEvent event_) {
        Gateway gateway = getGateway();
        if (null != gateway) {
            return;
        }
        Integer id = getGatewayId();
        GatewayContext gatewayContext = getGatewayContext();
        if (null != id) {
            gateway = getGatewayContext().getGateway(id);
        } else {
            gateway = gatewayContext.newGateway(getGatewayModel());
        }
        setGateway(gateway);
        setSettingProperties(gateway);

    }

    private void setSettingProperties(Gateway gateway) {
        if (getCurrentSettingSet() != null) {
            // it's already set
            return;
        }
        SettingSet root = (SettingSet) gateway.getSettings();
        if (root == null) {
            // no settings for this gateway
            return;
        }
        String currentSettingSetName = getCurrentSettingSetName();
        SettingSet currentSettingSet;
        if (StringUtils.isBlank(currentSettingSetName)) {
            currentSettingSet = (SettingSet) root.getDefaultSetting(SettingSet.class);
        } else {
            currentSettingSet = (SettingSet) root.getSetting(currentSettingSetName);
        }
        setCurrentSettingSet(currentSettingSet);
    }

    public void saveGateway() {
        Gateway gateway = getGateway();
        GatewayContext gatewayContext = getGatewayContext();
        gatewayContext.storeGateway(gateway);
        // attach gateway to current rule
        Integer ruleId = getRuleId();
        if (null != ruleId) {
            DialPlanContext manager = getDialPlanContext();
            DialingRule rule = manager.getRule(ruleId);
            rule.addGateway(gateway);
            manager.storeRule(rule);
        }
        // refresh gateway - it cannot be new any more
        if (getGatewayId() == null) {
            setGatewayId(gateway.getId());
            setGateway(null);
        }
    }

    public static EditGateway getEditPage(IRequestCycle cycle, Integer gatewayId,
            IPage returnPage, Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(null);
        page.setGatewayId(gatewayId);
        page.setGateway(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        return page;
    }

    public static EditGateway getAddPage(IRequestCycle cycle, PhoneModel model, IPage returnPage,
            Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(model);
        page.setGatewayId(null);
        page.setGateway(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        return page;
    }
}
