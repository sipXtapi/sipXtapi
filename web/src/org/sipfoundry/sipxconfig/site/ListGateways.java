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

import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    /**
     * When user clicks on link to edit a gateway
     */

    public void addGateway(IRequestCycle cycleTemp) {
        List gateways = getDialPlanManager().getGateways();

        final Gateway gateway = new Gateway();
        gateway.setName("Gateway");
        gateways.add(gateway);
    }
}
