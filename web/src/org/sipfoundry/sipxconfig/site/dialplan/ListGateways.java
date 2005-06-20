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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "ListGateways";

    // virtual properties
    public abstract GatewayContext getGatewayContext();

    public abstract Collection getSelectedRows();

    /**
     * When user clicks on link to edit a gateway
     */
    public void addGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage(EditGateway.PAGE);
        page.setGatewayId(null);
        page.setRuleId(null);
        page.setNextPage(cycle.getPage().getPageName());
        cycle.activate(page);
    }

    public void formSubmit(IRequestCycle cycle_) {
        Collection selectedRows = getSelectedRows();
        if (selectedRows != null) {
            getGatewayContext().deleteGateways(selectedRows);
        }
    }
}
