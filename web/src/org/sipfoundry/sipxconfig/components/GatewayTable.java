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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;

import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.site.EditGateway;

/**
 * GatewayTable
 */
public abstract class GatewayTable extends BaseComponent {
    public abstract Gateway getCurrentRow();

    public abstract String getEditPageName();

    /**
     * When user clicks on link to edit a gateway
     */
    public void edit(IRequestCycle cycle) {
        String editPageName = getEditPageName();
        EditGateway page = (EditGateway) cycle.getPage(editPageName);

        Gateway currentRow = getCurrentRow();
        page.setAddMode(false);
        page.setGateway(currentRow);
        cycle.activate(page);
    }
}
