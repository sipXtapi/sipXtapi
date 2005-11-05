/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ListBridges extends BasePage {

    public static final String PAGE = "ListBridges";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public void add(IRequestCycle cycle) {
        EditBridge editBridge = (EditBridge) cycle.getPage(EditBridge.PAGE);
        editBridge.activate(cycle, new PageCallback(this), null);
    }

    public void edit(IRequestCycle cycle) {
        Integer bridgeId = TapestryUtils.getBeanId(cycle);
        EditBridge editBridge = (EditBridge) cycle.getPage(EditBridge.PAGE);
        editBridge.activate(cycle, new PageCallback(this), bridgeId);
    }
}
