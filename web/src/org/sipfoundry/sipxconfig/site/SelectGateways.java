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
package org.sipfoundry.sipxconfig.site;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class SelectGateways extends BasePage implements PageRenderListener {
    public static final String PAGE = "SelectGateways";

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract Integer getDialPlanId();

    public abstract void setDialPlanId(Integer id);

    public abstract void setEmergencyGateway(boolean emergency);

    public abstract boolean getEmergencyGateway();

    public abstract Collection getSelectedRows();

    public abstract void setGateways(Collection gateways);

    public abstract Collection getGateways();

    public void pageBeginRender(PageEvent event_) {
        Collection gateways = getGateways();
        if (null == gateways) {
            gateways = getDialPlanManager().getAvailableGateways(getDialPlanId(), getEmergencyGateway());
            setGateways(gateways);
        }
    }

    public void formSubmit(IRequestCycle cycle) {
        Collection selectedRows = getSelectedRows();
        if (selectedRows != null) {
            selectGateways(selectedRows, getEmergencyGateway());
        }
        EditDialPlan editDialPlanPage = (EditDialPlan) cycle.getPage(EditDialPlan.PAGE);
        editDialPlanPage.setDialPlanId(getDialPlanId());
        cycle.activate(editDialPlanPage);
    }

    /**
     * Adds/removes gateways from dial plan
     * 
     * @param gateways list of gateway ids to be added to the dial plan
     * @param emergency true for emergency gateways
     */
    private void selectGateways(Collection gateways, boolean emergency) {
        DialPlanManager manager = getDialPlanManager();
        DialPlan plan = manager.getDialPlan(getDialPlanId());
        if (null != plan) {
            for (Iterator i = gateways.iterator(); i.hasNext();) {
                Integer id = (Integer) i.next();
                Gateway gateway = manager.getGateway(id);
                if (null != gateway) {
                    plan.addGateway(gateway, emergency);
                }
            }
        }
    }
}
