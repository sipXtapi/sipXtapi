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
 * EditGateway
 */
public abstract class EditGateway extends BasePage {
    private static final String PAGE_LIST_GATEWAYS = "ListGateways";

    public abstract void setGateway(Gateway gateway);

    public abstract Gateway getGateway();

    public abstract void setAddMode(boolean addMode);

    public abstract boolean getAddMode();

    public abstract DialPlanManager getDialPlanManager();

    public void save(IRequestCycle cycle) {
        DialPlanManager manager = getDialPlanManager();
        Gateway gateway = getGateway();
        if (getAddMode()) {
            manager.addGateway(gateway);
        } else {
            manager.updateGateway(gateway);
        }
        cycle.activate(PAGE_LIST_GATEWAYS);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(PAGE_LIST_GATEWAYS);
    }
}
