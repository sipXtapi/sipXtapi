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
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;


public abstract class EditLine extends BasePage implements PageRenderListener {

    public static final String PAGE = "EditLine";

    public abstract Line getLine();
    
    public abstract void setLine(Line line);
    
    /** REQUIRED PROPERTY */
    public abstract int getLineId();
    
    public abstract void setLineId(int id);
    
    public abstract PhoneContext getPhoneContext();

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        PhoneContext dao = getPhoneContext();
        dao.storeLine(getLine());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setLine(context.loadLine(getLineId()));
    }
}
