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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditDialPlan extends BasePage implements PageRenderListener {

    private static final String NEXT_PAGE = "EditDialPlan";

    // virtual properties
    public abstract DialPlan getDialPlan();

    public abstract void setDialPlan(DialPlan plan);

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {

        DialPlan plan = getDialPlan();
        if (plan == null) {
            plan = new DialPlan("kuku", 5);
            setDialPlan(plan);
        }
    }

    public void save(IRequestCycle cycle) {
        cycle.activate(NEXT_PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(NEXT_PAGE);
    }
}
