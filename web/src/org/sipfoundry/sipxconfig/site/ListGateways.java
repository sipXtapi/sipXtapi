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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "ListGateways";

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract Gateway getCurrentRow();

    /**
     * When user clicks on link to edit a gateway
     */

    public void addGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage(EditGateway.PAGE);
        page.setGatewayId(null);
        page.setCurrentDialPlanId(null);
        cycle.activate(page);
    }

    public void editGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage(EditGateway.PAGE);
        Gateway currentRow = getCurrentRow();
        page.setGatewayId(currentRow.getId());
        page.setCurrentDialPlanId(null);
        cycle.activate(page);
    }
}
