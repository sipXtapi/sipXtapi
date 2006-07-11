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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;


public abstract class EditLine extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "EditLine";

    public abstract Line getLine();
    
    public abstract void setLine(Line line);
    
    /** REQUIRED PROPERTY */
    public abstract Integer getLineId();
    
    public abstract void setLineId(Integer id);
    
    public abstract PhoneContext getPhoneContext();

    public String ok() {
        apply();
        return ManagePhones.PAGE;
    }

    public void apply() {
        PhoneContext dao = getPhoneContext();
        dao.storeLine(getLine());
        dao.flush();
    }
    
    public String cancel() {
        return ManagePhones.PAGE;
    }
    
    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setLine(context.loadLine(getLineId()));
    }
}
