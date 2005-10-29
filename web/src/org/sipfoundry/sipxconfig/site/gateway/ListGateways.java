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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.gateway.GatewayManager;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "ListGateways";

    // virtual properties
    public abstract GatewayManager getGatewayContext();

    public abstract Collection getGatewaysToDelete();

    public abstract Collection getGatewaysToPropagate();

    /**
     * When user clicks on link to edit a gateway
     */
    public void addGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage(EditGateway.PAGE);
        page.setGatewayId(null);
        page.setRuleId(null);
        page.setCallback(new PageCallback(this));
        cycle.activate(page);
    }

    public void formSubmit(IRequestCycle cycle_) {
        Collection selectedRows = getGatewaysToDelete();
        if (selectedRows != null) {
            getGatewayContext().deleteGateways(selectedRows);
        }
        selectedRows = getGatewaysToPropagate();
        if (selectedRows != null) {
            getGatewayContext().propagateGateways(selectedRows);
        }
    }

    public void propagateAllGateways(IRequestCycle cycle_) {
        getGatewayContext().propagateAllGateways();
    }
}
