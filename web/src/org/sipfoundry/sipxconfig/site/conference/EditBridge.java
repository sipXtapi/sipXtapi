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

import java.io.Serializable;
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class EditBridge extends PageWithCallback implements PageRenderListener {
    public static final String PAGE = "EditBridge";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract Serializable getBridgeId();

    public abstract void setBridgeId(Serializable id);

    public abstract Bridge getBridge();

    public abstract void setBridge(Bridge acdServer);

    public abstract boolean getChanged();

    public void pageBeginRender(PageEvent event_) {
        if (getBridge() != null) {
            return;
        }
        Bridge bridge = null;
        if (getBridgeId() != null) {
            bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
        } else {
            // take the first bridge if available
            List bridges = getConferenceBridgeContext().getBridges();
            if (bridges.size() > 0) {
                bridge = (Bridge) bridges.get(0);
            } else {
                bridge = getConferenceBridgeContext().newBridge();
            }
            setBridgeId(bridge.getId());
        }
        setBridge(bridge);
    }

    public void apply(IRequestCycle cycle) {
        if (TapestryUtils.isValid(this)) {
            saveValid(cycle);
        }
    }

    private void saveValid(IRequestCycle cycle_) {
        Bridge bridge = getBridge();
        boolean isNew = bridge.isNew();
        getConferenceBridgeContext().store(bridge);
        if (isNew) {
            Integer id = bridge.getId();
            setBridgeId(id);
        }
    }

    public void formSubmit(IRequestCycle cycle_) {
        if (getChanged()) {
            setBridge(null);
        }
    }

    public void addConference(IRequestCycle cycle) {
        apply(cycle);
        EditConference editConference = (EditConference) cycle.getPage(EditConference.PAGE);
        editConference.activate(cycle, new PageCallback(this), getBridgeId(), null);
    }

    public void editConference(IRequestCycle cycle) {
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        EditConference editConference = (EditConference) cycle.getPage(EditConference.PAGE);
        editConference.activate(cycle, new PageCallback(this), getBridgeId(), id);
    }
}
