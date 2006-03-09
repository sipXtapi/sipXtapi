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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
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

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract String getCurrentSettingSetName();

    public abstract void setCurrentSettingSet(SettingSet currentSettingSet);

    private boolean isValid() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        return !delegate.getHasErrors();
    }

    public void apply() {
        if (isValid()) {
            saveGateway();
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Gateway gateway = getGateway();
        if (null != gateway) {
            return;
        }
        Integer id = getGatewayId();
        if (null != id) {
            gateway = getGatewayContext().getGateway(id);
        } else {
            gateway = new Gateway();
        }
        setGateway(gateway);

        // settings part
        SettingSet root = (SettingSet) gateway.getSettings();
        if (root != null) {
            String currentSettingSetName = getCurrentSettingSetName();
            SettingSet currentSettingSet;
            if (StringUtils.isBlank(currentSettingSetName)) {
                currentSettingSet = (SettingSet) root.getDefaultSetting(SettingSet.class);
            } else {
                currentSettingSet = (SettingSet) root.getSetting(currentSettingSetName);
            }
            setCurrentSettingSet(currentSettingSet);
        }
    }

    void saveGateway() {
        Gateway gateway = getGateway();
        GatewayContext gatewayContext = getGatewayContext();
        if (gateway.isNew()) {
            Gateway newGateway = gatewayContext.newGateway(gateway.getModel());
            newGateway.setName(gateway.getName());
            newGateway.setAddress(gateway.getAddress());
            newGateway.setDescription(gateway.getDescription());
            newGateway.setSerialNumber(gateway.getSerialNumber());
            setGateway(newGateway);
            gateway = newGateway;
        }
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
}
