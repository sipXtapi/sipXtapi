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
import java.util.Collections;
import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.components.SelectMap;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage implements PageRenderListener {
    public static final String PAGE = "ListGateways";

    private Collection m_selectedRows = Collections.EMPTY_LIST;

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract Gateway getCurrentRow();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public void pageBeginRender(PageEvent event_) {
        SelectMap selections = getSelections();
        if (null == selections) {
            setSelections(new SelectMap());
        }
    }

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

    public void delete(IRequestCycle cycle_) {
        m_selectedRows = getSelections().getAllSelected();
    }

    public void formSubmit(IRequestCycle cycle_) {
        DialPlanManager manager = getDialPlanManager();
        for (Iterator i = m_selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            manager.deleteGateway(id);
        }
        m_selectedRows = Collections.EMPTY_LIST;
    }
}
