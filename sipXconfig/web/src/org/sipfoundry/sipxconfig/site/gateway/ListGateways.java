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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.ProfileManager;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "ListGateways";

    public abstract GatewayContext getGatewayContext();

    public abstract ProfileManager getGatewayProfileManager();

    public abstract Collection getGatewaysToDelete();

    public abstract Collection getGatewaysToPropagate();

    /**
     * When user clicks on link to edit a gateway
     */
    public IPage addGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage(EditGateway.PAGE);
        page.setGatewayId(null);
        page.setRuleId(null);
        page.setReturnPage(this);
        return page;
    }

    public void formSubmit() {
        Collection selectedRows = getGatewaysToDelete();
        if (selectedRows != null) {
            getGatewayContext().deleteGateways(selectedRows);
        }
        selectedRows = getGatewaysToPropagate();
        if (selectedRows != null) {
            propagateGateways(selectedRows);
        }
    }

    public void propagateAllGateways() {
        Collection gatewayIds = getGatewayContext().getAllGatewayIds();
        propagateGateways(gatewayIds);
    }

    private void propagateGateways(Collection gatewayIds) {
        getGatewayProfileManager().generateProfilesAndRestart(gatewayIds);
        String msg = getMessages().format("msg.success.profiles",
                Integer.toString(gatewayIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }
}
