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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlanContext;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class EditFlexibleDialPlan extends BasePage {
    public static final String PAGE = "EditFlexibleDialPlan";

    // virtual properties
    public abstract DialPlanContext getDialPlanManager();

    public abstract DialingRule getCurrentRow();

    public abstract void setCurrentRow(DialingRule plan);

    public abstract Collection getSelectedRows();

    public abstract Collection getRowsToDuplicate();

    public void add(IRequestCycle cycle) {
        cycle.activate(SelectRuleType.PAGE);
    }

    public void edit(IRequestCycle cycle) {
        SelectRuleType.activateEditPage(getCurrentRow(), cycle);
    }

    public void formSubmit(IRequestCycle cycle_) {
        delete();
        duplicate();
    }

    public void activate(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanManager();
        manager.generateDialPlan();
        cycle.activate(ActivateDialPlan.PAGE);
    }

    /**
     * Deletes all selected rows (on this screen deletes rules from flexible
     * dial plan).
     */
    private void delete() {
        Collection selectedRows = getSelectedRows();
        if (null != selectedRows) {
            FlexibleDialPlanContext manager = getDialPlanManager().getFlexDialPlan();
            manager.deleteRules(selectedRows);
        }
    }

    /**
     * Deletes all selected rows (on this screen deletes rules from flexible
     * dial plan).
     */
    private void duplicate() {
        Collection selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            FlexibleDialPlanContext manager = getDialPlanManager().getFlexDialPlan();
            manager.duplicateRules(selectedRows);
        }
    }
}
