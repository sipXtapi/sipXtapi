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

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    /**
     * When user clicks on link to edit a gateway
     */

    public void addGateway(IRequestCycle cycle) {
        EditGateway page = (EditGateway) cycle.getPage("EditGateway");
        page.setAddMode(true);
        cycle.activate(page);
    }
}
