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

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage implements PageRenderListener {
    public static final String PAGE = "ListGateways";

    private List m_selectedRows = Collections.EMPTY_LIST;

    // virtual properties
    public abstract DialPlanManager getDialPlanManager();

    public abstract Gateway getCurrentRow();

    public abstract Map getSelected();

    public abstract void setSelected(Map selected);

    public void pageBeginRender(PageEvent event_) {
        Map selected = getSelected();
        if (null == selected) {
            DialPlanManager dialPlanManager = getDialPlanManager();
            List gateways = dialPlanManager.getGateways();

            selected = new HashMap(gateways.size());
            for (Iterator i = gateways.iterator(); i.hasNext();) {
                Gateway gateway = (Gateway) i.next();
                selected.put(gateway.getId(), new Boolean(false));
            }
            setSelected(selected);
        }
    }

    public Boolean getMarked(Integer id) {
        return (Boolean) getSelected().get(id);
    }

    public void setMarked(Integer id, Boolean state) {
        getSelected().put(id, state);
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
        m_selectedRows = new ArrayList();
        Map m = getSelected();
        for (Iterator i = m.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            if (entry.getValue().equals(Boolean.TRUE)) {
                m_selectedRows.add(entry.getKey());
            }
        }
    }

    public void formSubmit(IRequestCycle cycle_) {
        DialPlanManager manager = getDialPlanManager();
        for (Iterator i = m_selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            manager.deleteGateway(id);
        }
        m_selectedRows.clear();
    }
}
