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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListDialPlans extends BasePage {
    public static final String PAGE = "ListDialPlans";

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract DialPlan getCurrentRow();

    public abstract void setCurrentRow(DialPlan plan);

    public abstract Collection getSelectedRows();

    /**
     * When user clicks on link to edit a gateway
     */

    public void add(IRequestCycle cycle) {
        EditDialPlan page = (EditDialPlan) cycle.getPage(EditDialPlan.PAGE);
        page.setDialPlanId(null);
        cycle.activate(page);
    }

    public void edit(IRequestCycle cycle) {
        EditDialPlan page = (EditDialPlan) cycle.getPage(EditDialPlan.PAGE);
        DialPlan currentRow = getCurrentRow();
        page.setDialPlanId(currentRow.getId());
        cycle.activate(page);
    }

    public void formSubmit(IRequestCycle cycle_) {
        Collection selectedRows = getSelectedRows();
        if (selectedRows != null) {
            DialPlanManager manager = getDialPlanManager();
            manager.deleteDialPlans(selectedRows);
        }
    }
}
